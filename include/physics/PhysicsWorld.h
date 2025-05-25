#pragma once
#include "raylib.h"
#include <vector>

struct PhysicsBody {
    Vector3 position;
    Vector3 velocity;
    Vector3 acceleration;
    Vector3 colliderSize;  // Size of the collider for this body
    float mass;
    bool isGrounded;
    bool useGravity;
    
    PhysicsBody(Vector3 pos = {0.0f, 0.0f, 0.0f}, float m = 1.0f, Vector3 size = {1.0f, 1.0f, 1.0f})
        : position(pos), velocity({0.0f, 0.0f, 0.0f}), acceleration({0.0f, 0.0f, 0.0f}), 
          colliderSize(size), mass(m), isGrounded(false), useGravity(true) {}
};

struct Collider {
    Vector3 position;
    Vector3 size;
    bool isStatic;
    
    Collider(Vector3 pos = {0.0f, 0.0f, 0.0f}, Vector3 sz = {1.0f, 1.0f, 1.0f}, bool stat = false)
        : position(pos), size(sz), isStatic(stat) {}
};

class PhysicsWorld {
private:
    Vector3 gravity;
    float deltaTime;
    int groundedFrameStability;  // Número de frames para mantener el estado isGrounded
    
public:
    PhysicsWorld(Vector3 grav = {0.0f, -9.81f, 0.0f});
    ~PhysicsWorld();
    
    // Physics simulation
    void Update(float dt);
    void ApplyGravity(PhysicsBody& body);
    void UpdatePhysicsBody(PhysicsBody& body);
    bool IsBodySupported(const PhysicsBody& body, const std::vector<Collider*>& staticColliders, const std::vector<PhysicsBody*>& dynamicBodies);
    
    // Collision detection - métodos originales para compatibilidad
    bool CheckCollision(const Collider& a, const Collider& b);
    bool CheckCollisionAABB(Vector3 posA, Vector3 sizeA, Vector3 posB, Vector3 sizeB);
    
    // Nuevos métodos de colisión basados en BoundingBox de raylib
    BoundingBox GetBoundingBox(const Vector3& position, const Vector3& size);
    bool CheckCollisionBoxes(const PhysicsBody& bodyA, const PhysicsBody& bodyB);
    bool CheckCollisionBoxFloor(const PhysicsBody& body, const Collider& floor, float* penetrationDepth = nullptr);
    
    // Resolución de colisiones
    void ResolveCollision(PhysicsBody& body, const Collider& staticCollider);
    void ResolveCubeCollision(PhysicsBody& bodyA, PhysicsBody& bodyB);
    
    // Getters/Setters
    void SetGravity(Vector3 grav) { gravity = grav; }
    Vector3 GetGravity() const { return gravity; }
};
