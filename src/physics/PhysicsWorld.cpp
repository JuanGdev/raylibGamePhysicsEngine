#include "physics/PhysicsWorld.h"
#include "raymath.h"
#include <algorithm>

PhysicsWorld::PhysicsWorld(Vector3 grav) : gravity(grav), deltaTime(0.0f) {
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
    // Reset grounded state at the beginning of each frame
    // It will be set to true if a collision is detected
    body.isGrounded = false;
    
    // Apply gravity
    ApplyGravity(body);
    
    // Update velocity
    Vector3 deltaVelocity = Vector3Scale(body.acceleration, deltaTime);
    body.velocity = Vector3Add(body.velocity, deltaVelocity);
    
    // Update position
    Vector3 deltaPosition = Vector3Scale(body.velocity, deltaTime);
    body.position = Vector3Add(body.position, deltaPosition);
    
    // Reset acceleration for next frame
    body.acceleration = {0.0f, 0.0f, 0.0f};
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

void PhysicsWorld::ResolveCollision(PhysicsBody& body, const Collider& staticCollider) {
    // Use the actual collider size from the body
    Vector3 bodyHalfSize = Vector3Scale(body.colliderSize, 0.5f);
    Vector3 colliderHalfSize = Vector3Scale(staticCollider.size, 0.5f);
    
    // Calculate overlap on each axis
    float overlapX = (bodyHalfSize.x + colliderHalfSize.x) - fabs(body.position.x - staticCollider.position.x);
    float overlapY = (bodyHalfSize.y + colliderHalfSize.y) - fabs(body.position.y - staticCollider.position.y);
    float overlapZ = (bodyHalfSize.z + colliderHalfSize.z) - fabs(body.position.z - staticCollider.position.z);
    
    // Add small tolerance to prevent jitter
    const float tolerance = 0.001f;
    
    // Find the smallest overlap (the axis of least penetration)
    if (overlapY > 0 && overlapY <= overlapX && overlapY <= overlapZ) {
        // Resolve Y collision (floor/ceiling)
        if (body.position.y > staticCollider.position.y) {
            // Object is above the collider
            body.position.y = staticCollider.position.y + colliderHalfSize.y + bodyHalfSize.y + tolerance;
            body.isGrounded = true;
            if (body.velocity.y < 0) {
                body.velocity.y = 0; // Stop downward velocity
            }
        } else {
            // Object is below the collider
            body.position.y = staticCollider.position.y - colliderHalfSize.y - bodyHalfSize.y - tolerance;
            if (body.velocity.y > 0) {
                body.velocity.y = 0; // Stop upward velocity
            }
        }
    }
}
