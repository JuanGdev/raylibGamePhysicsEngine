#include "physics/PhysicsWorld.h"
#include "raymath.h"
#include <algorithm>

PhysicsWorld::PhysicsWorld(Vector3 grav) : gravity(grav), deltaTime(0.0f), groundedFrameStability(3) {
    // Inicializamos con 3 frames de estabilidad para el estado grounded
}

PhysicsWorld::~PhysicsWorld() {
    // Nothing to cleanup for now
}

void PhysicsWorld::Update(float dt) {
    deltaTime = dt;
}

void PhysicsWorld::ApplyGravity(PhysicsBody& body) {
    if (body.useGravity && !body.isGrounded) {
        body.acceleration = Vector3Add(body.acceleration, Vector3Scale(gravity, 1.0f / body.mass));
    }
}

void PhysicsWorld::UpdatePhysicsBody(PhysicsBody& body) {
    // Store previous grounded state
    bool wasGrounded = body.isGrounded;
    
    // Apply gravity (only if not grounded)
    ApplyGravity(body);
    
    // Update velocity with acceleration
    Vector3 deltaVelocity = Vector3Scale(body.acceleration, deltaTime);
    body.velocity = Vector3Add(body.velocity, deltaVelocity);
    
    // Aplicar amortiguamiento general para reducir la vibración
    const float dampingFactor = 0.98f;
    body.velocity = Vector3Scale(body.velocity, dampingFactor);
    
    // Zero out very small velocities when on the ground to prevent micro-movements
    if (wasGrounded) {
        // Umbral más estricto para velocidades verticales cuando está en el suelo
        const float verticalThreshold = 0.005f;
        if (fabs(body.velocity.y) < verticalThreshold) {
            body.velocity.y = 0.0f;
        }
        
        // También reducir velocidades horizontales muy pequeñas para simular fricción
        const float horizontalThreshold = 0.01f;
        if (fabs(body.velocity.x) < horizontalThreshold) body.velocity.x = 0.0f;
        if (fabs(body.velocity.z) < horizontalThreshold) body.velocity.z = 0.0f;
    }
    
    // Update position
    Vector3 deltaPosition = Vector3Scale(body.velocity, deltaTime);
    body.position = Vector3Add(body.position, deltaPosition);
    
    // Reset acceleration for next frame
    body.acceleration = {0.0f, 0.0f, 0.0f};
    
    // Note: We don't reset isGrounded here anymore - it will be updated by collision checks
    // pero mantendremos el estado grounded durante algunos frames para estabilidad
    if (!wasGrounded) {
        // Si acabamos de perder el estado grounded, lo reseteamos
        body.isGrounded = false;
    }
}

bool PhysicsWorld::CheckCollision(const Collider& a, const Collider& b) {
    return CheckCollisionAABB(a.position, a.size, b.position, b.size);
}

bool PhysicsWorld::CheckCollisionAABB(Vector3 posA, Vector3 sizeA, Vector3 posB, Vector3 sizeB) {
    // Calculate half sizes
    Vector3 halfSizeA = Vector3Scale(sizeA, 0.5f);
    Vector3 halfSizeB = Vector3Scale(sizeB, 0.5f);
    
    // Check collision on all axes
    bool collisionX = (posA.x - halfSizeA.x) <= (posB.x + halfSizeB.x) && 
                      (posA.x + halfSizeA.x) >= (posB.x - halfSizeB.x);
    bool collisionY = (posA.y - halfSizeA.y) <= (posB.y + halfSizeB.y) && 
                      (posA.y + halfSizeA.y) >= (posB.y - halfSizeB.y);
    bool collisionZ = (posA.z - halfSizeA.z) <= (posB.z + halfSizeB.z) && 
                      (posA.z + halfSizeA.z) >= (posB.z - halfSizeB.z);
    
    return collisionX && collisionY && collisionZ;
}

// Nuevas funciones de colisión

BoundingBox PhysicsWorld::GetBoundingBox(const Vector3& position, const Vector3& size) {
    Vector3 halfSize = Vector3Scale(size, 0.5f);
    
    return (BoundingBox){
        // Esquina inferior (mínimos)
        (Vector3){ position.x - halfSize.x, position.y - halfSize.y, position.z - halfSize.z },
        // Esquina superior (máximos)
        (Vector3){ position.x + halfSize.x, position.y + halfSize.y, position.z + halfSize.z }
    };
}

bool PhysicsWorld::CheckCollisionBoxes(const PhysicsBody& bodyA, const PhysicsBody& bodyB) {
    BoundingBox boxA = GetBoundingBox(bodyA.position, bodyA.colliderSize);
    BoundingBox boxB = GetBoundingBox(bodyB.position, bodyB.colliderSize);
    
    return ::CheckCollisionBoxes(boxA, boxB);  // Uso del operador de ámbito para usar la función de raylib
}

bool PhysicsWorld::CheckCollisionBoxFloor(const PhysicsBody& body, const Collider& floor, float* penetrationDepth) {
    BoundingBox bodyBox = GetBoundingBox(body.position, body.colliderSize);
    BoundingBox floorBox = GetBoundingBox(floor.position, floor.size);
    
    bool collision = ::CheckCollisionBoxes(bodyBox, floorBox);  // Uso del operador de ámbito para usar la función de raylib
    
    // Si hay colisión y se solicitó la profundidad de penetración
    if (collision && penetrationDepth != nullptr) {
        float bodyBottom = bodyBox.min.y;
        float floorTop = floorBox.max.y;
        
        // Solo calcular penetración si el cuerpo está por encima del suelo
        if (body.position.y > floor.position.y) {
            *penetrationDepth = floorTop - bodyBottom;
        } else {
            *penetrationDepth = 0.0f;
        }
    }
    
    return collision;
}

void PhysicsWorld::ResolveCollision(PhysicsBody& body, const Collider& staticCollider) {
    // Usamos el nuevo sistema de BoundingBox para detección de colisiones
    float penetrationDepth = 0.0f;
    bool collision = CheckCollisionBoxFloor(body, staticCollider, &penetrationDepth);
    
    // Si no hay colisión, solo reseteamos el estado cuando el contador es cero
    if (!collision && body.isGrounded) {
        // Implementamos histéresis para estabilizar el estado grounded
        // En lugar de cambiar el estado instantáneamente, usamos un contador de frames
        static int groundedCounter = 0;
        
        // Si ya no estamos detectando colisión con el suelo
        if (++groundedCounter >= groundedFrameStability) {
            body.isGrounded = false;
            groundedCounter = 0;
        }
        return;
    }
    
    // Resetear el contador si volvimos a detectar colisión
    if (collision) {
        // Determinar qué tipo de colisión es (suelo, techo o pared)
        BoundingBox bodyBox = GetBoundingBox(body.position, body.colliderSize);
        BoundingBox colliderBox = GetBoundingBox(staticCollider.position, staticCollider.size);
        
        // Calcular el vector de centro a centro
        Vector3 bodyCenter = {
            (bodyBox.min.x + bodyBox.max.x) * 0.5f,
            (bodyBox.min.y + bodyBox.max.y) * 0.5f,
            (bodyBox.min.z + bodyBox.max.z) * 0.5f
        };
        
        Vector3 colliderCenter = {
            (colliderBox.min.x + colliderBox.max.x) * 0.5f,
            (colliderBox.min.y + colliderBox.max.y) * 0.5f,
            (colliderBox.min.z + colliderBox.max.z) * 0.5f
        };
        
        // Vector desde el colisionador al cuerpo
        Vector3 direction = Vector3Subtract(bodyCenter, colliderCenter);
        
        // Para el caso del suelo, que es muy plano, consideramos primero la posición Y
        // antes de determinar el eje principal de colisión
        
        // Verificar si este objeto está sobre el suelo por su posición relativa
        bool isAboveObject = (bodyCenter.y > colliderCenter.y);
        
        // Si el colisionador es muy plano (como nuestro suelo) y el cuerpo está encima
        bool isFlatCollider = staticCollider.size.y < staticCollider.size.x * 0.25f && 
                              staticCollider.size.y < staticCollider.size.z * 0.25f;
                              
        if (isFlatCollider && isAboveObject && fabsf(direction.y) > 0.01f) {
            // Tratamos como colisión vertical ya que es el suelo
            
            // Es una colisión con el suelo (cuerpo por encima del colisionador)
            body.isGrounded = true;
            
            // Colocar el objeto justo encima del suelo con un pequeño margen
            float floorHeight = colliderBox.max.y;
            float bodyBottom = bodyBox.min.y;
            
            // Corregir la posición si hay penetración
            if (bodyBottom < floorHeight) {
                body.position.y += (floorHeight - bodyBottom + 0.001f);
            }
            
            // Detener caída
            if (body.velocity.y < 0) {
                body.velocity.y = 0;
            }
            
            // Aplicar fricción horizontal cuando está en el suelo
            const float frictionFactor = 0.92f;
            body.velocity.x *= frictionFactor;
            body.velocity.z *= frictionFactor;
            
            return; // Terminamos la resolución aquí, ya que es una colisión con el suelo
        }
        
        // Para otros tipos de colisión, seguimos con el algoritmo normal
        // Calcular la penetración en cada dirección
        float absX = fabsf(direction.x);
        float absY = fabsf(direction.y);
        float absZ = fabsf(direction.z);
        
        // Determinar el eje de menor penetración
        if (absY > absX && absY > absZ) {
            // Colisión vertical (suelo o techo)
            if (direction.y > 0) {
                // Es una colisión con el suelo (cuerpo por encima del colisionador)
                
                // Set grounded state with stability counter
                body.isGrounded = true;
                
                // Colocar el objeto justo encima del suelo con un pequeño margen
                float floorHeight = colliderBox.max.y;
                float bodyBottom = bodyBox.min.y;
                
                // Corregir la posición si hay penetración
                if (bodyBottom < floorHeight) {
                    body.position.y += (floorHeight - bodyBottom + 0.001f);
                }
                
                // Detener caída
                if (body.velocity.y < 0) {
                    body.velocity.y = 0;
                }
                
                // Aplicar fricción horizontal cuando está en el suelo
                const float frictionFactor = 0.92f;
                body.velocity.x *= frictionFactor;
                body.velocity.z *= frictionFactor;
            } 
            else {
                // Colisión con techo (cuerpo por debajo del colisionador)
                float ceilingHeight = colliderBox.min.y;
                float bodyTop = bodyBox.max.y;
                
                if (bodyTop > ceilingHeight) {
                    body.position.y -= (bodyTop - ceilingHeight + 0.001f);
                }
                
                // Detener movimiento hacia arriba
                if (body.velocity.y > 0) {
                    body.velocity.y = 0;
                }
            }
        } 
        else if (absX >= absZ) {
            // Colisión lateral en X (izquierda/derecha)
            float sign = (direction.x > 0) ? 1.0f : -1.0f;
            
            if (sign > 0) {
                // Colisión con el lado izquierdo
                body.position.x = colliderBox.max.x + (bodyBox.max.x - bodyBox.min.x) * 0.5f + 0.001f;
            } else {
                // Colisión con el lado derecho
                body.position.x = colliderBox.min.x - (bodyBox.max.x - bodyBox.min.x) * 0.5f - 0.001f;
            }
            
            // Detener movimiento horizontal
            body.velocity.x = 0;
        } 
        else {
            // Colisión lateral en Z (adelante/atrás)
            float sign = (direction.z > 0) ? 1.0f : -1.0f;
            
            if (sign > 0) {
                // Colisión con el lado frontal
                body.position.z = colliderBox.max.z + (bodyBox.max.z - bodyBox.min.z) * 0.5f + 0.001f;
            } else {
                // Colisión con el lado trasero
                body.position.z = colliderBox.min.z - (bodyBox.max.z - bodyBox.min.z) * 0.5f - 0.001f;
            }
            
            // Detener movimiento en Z
            body.velocity.z = 0;
        }
    }
}

void PhysicsWorld::ResolveCubeCollision(PhysicsBody& bodyA, PhysicsBody& bodyB) {
    // Usamos BoundingBox para detección de colisiones más precisa
    BoundingBox boxA = GetBoundingBox(bodyA.position, bodyA.colliderSize);
    BoundingBox boxB = GetBoundingBox(bodyB.position, bodyB.colliderSize);
    
    // Verificar si hay colisión
    bool collision = ::CheckCollisionBoxes(boxA, boxB);
    if (!collision) return;
    
    // Calcular centros
    Vector3 centerA = {
        (boxA.min.x + boxA.max.x) * 0.5f,
        (boxA.min.y + boxA.max.y) * 0.5f,
        (boxA.min.z + boxA.max.z) * 0.5f
    };
    
    Vector3 centerB = {
        (boxB.min.x + boxB.max.x) * 0.5f,
        (boxB.min.y + boxB.max.y) * 0.5f,
        (boxB.min.z + boxB.max.z) * 0.5f
    };
    
    // Vector de colisión (desde A hacia B)
    Vector3 collisionVector = Vector3Subtract(centerB, centerA);
    float distance = Vector3Length(collisionVector);
    
    // Evitar división por cero
    if (distance <= 0.0001f) {
        collisionVector = (Vector3){0.0f, 1.0f, 0.0f}; // Vector arbitrario hacia arriba
        distance = 1.0f;
    }
    
    // Normalizar vector de colisión
    Vector3 collisionNormal = Vector3Scale(collisionVector, 1.0f / distance);
    
    // Detección especial para colisiones verticales (cubo sobre cubo)
    bool isVerticalCollision = false;
    bool isUpperCubeA = false; // true si A está encima de B, false si B está encima de A
    
    // Si la distancia vertical es significativa y uno está encima del otro
    if (fabsf(collisionVector.y) > fabsf(collisionVector.x) && 
        fabsf(collisionVector.y) > fabsf(collisionVector.z)) {
        isVerticalCollision = true;
        isUpperCubeA = collisionVector.y < 0; // A está arriba si el vector va de A hacia abajo (a B)
        
        // Caso especial: un cubo está sobre otro
        if (isUpperCubeA) {
            // A está encima de B
            if (boxA.min.y <= boxB.max.y && boxA.min.y > centerB.y) {
                bodyA.isGrounded = true;
                // Posicionar A exactamente sobre B
                bodyA.position.y = boxB.max.y + bodyA.colliderSize.y * 0.5f + 0.001f;
                if (bodyA.velocity.y < 0) bodyA.velocity.y = 0;
            }
        } else {
            // B está encima de A
            if (boxB.min.y <= boxA.max.y && boxB.min.y > centerA.y) {
                bodyB.isGrounded = true;
                // Posicionar B exactamente sobre A
                bodyB.position.y = boxA.max.y + bodyB.colliderSize.y * 0.5f + 0.001f;
                if (bodyB.velocity.y < 0) bodyB.velocity.y = 0;
            }
        }
        
        // Si es una colisión vertical, aplicamos fricción horizontal
        if (isUpperCubeA && bodyA.isGrounded) {
            const float frictionFactor = 0.92f;
            bodyA.velocity.x *= frictionFactor;
            bodyA.velocity.z *= frictionFactor;
        } else if (!isUpperCubeA && bodyB.isGrounded) {
            const float frictionFactor = 0.92f;
            bodyB.velocity.x *= frictionFactor;
            bodyB.velocity.z *= frictionFactor;
        }
    }
    
    // Para colisiones no verticales o si necesitamos procesar física adicional
    if (!isVerticalCollision) {
        // Calcular velocidad relativa
        Vector3 relativeVelocity = Vector3Subtract(bodyB.velocity, bodyA.velocity);
        
        // Velocidad relativa a lo largo de la normal
        float velocityAlongNormal = Vector3DotProduct(relativeVelocity, collisionNormal);
        
        // No resolver si los objetos se están alejando
        if (velocityAlongNormal > 0) return;
        
        // Calcular restitución (rebote) - más bajo para reducir inestabilidad
        float restitution = 0.2f;  // Valor bajo para menos rebote
        
        // Calcular impulso escalar
        float impulseScalar = -(1.0f + restitution) * velocityAlongNormal;
        impulseScalar /= (1.0f / bodyA.mass + 1.0f / bodyB.mass);
        
        // Aplicar impulso
        Vector3 impulse = Vector3Scale(collisionNormal, impulseScalar);
        
        // Actualizar velocidades inversamente proporcional a las masas
        bodyA.velocity = Vector3Subtract(bodyA.velocity, Vector3Scale(impulse, 1.0f / bodyA.mass));
        bodyB.velocity = Vector3Add(bodyB.velocity, Vector3Scale(impulse, 1.0f / bodyB.mass));
        
        // Calcular tamaños
        Vector3 sizeA = Vector3Subtract(boxA.max, boxA.min);
        Vector3 sizeB = Vector3Subtract(boxB.max, boxB.min);
        
        // Calcular penetración para separar los objetos
        Vector3 extents = Vector3Scale(Vector3Add(sizeA, sizeB), 0.5f);
        Vector3 depth;
        depth.x = extents.x - fabsf(collisionVector.x);
        depth.y = extents.y - fabsf(collisionVector.y);
        depth.z = extents.z - fabsf(collisionVector.z);
        
        // Calcular eje con menor penetración
        float minDepth = depth.x;
        int minAxis = 0;
        
        if (depth.y < minDepth) {
            minDepth = depth.y;
            minAxis = 1;
        }
        
        if (depth.z < minDepth) {
            minDepth = depth.z;
            minAxis = 2;
        }
        
        // Separar los objetos a lo largo del eje de menor penetración
        Vector3 separation = {0, 0, 0};
        
        switch (minAxis) {
            case 0: // X axis
                separation.x = (collisionVector.x > 0 ? 1 : -1) * (minDepth + 0.001f);
                break;
            case 1: // Y axis
                separation.y = (collisionVector.y > 0 ? 1 : -1) * (minDepth + 0.001f);
                
                // Si la colisión es vertical, actualizar estado grounded
                if (collisionVector.y < 0 && boxA.min.y < boxB.max.y) {
                    // B está encima de A
                    bodyB.isGrounded = true;
                } else if (collisionVector.y > 0 && boxB.min.y < boxA.max.y) {
                    // A está encima de B
                    bodyA.isGrounded = true;
                }
                break;
            case 2: // Z axis
                separation.z = (collisionVector.z > 0 ? 1 : -1) * (minDepth + 0.001f);
                break;
        }
        
        // Separar los objetos en base a las masas
        float totalMass = bodyA.mass + bodyB.mass;
        float ratioA = bodyB.mass / totalMass;
        float ratioB = bodyA.mass / totalMass;
        
        bodyA.position = Vector3Subtract(bodyA.position, Vector3Scale(separation, ratioA));
        bodyB.position = Vector3Add(bodyB.position, Vector3Scale(separation, ratioB));
    }
}