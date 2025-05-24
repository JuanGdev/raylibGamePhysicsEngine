#pragma once
#include "raylib.h"
#include "physics/PhysicsWorld.h"

class GameObject {
private:
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;
    Color color;
    PhysicsBody* physicsBody;
    Collider* collider;
    bool hasPhysics;
    
public:
    GameObject(Vector3 pos = {0.0f, 0.0f, 0.0f}, 
               Vector3 rot = {0.0f, 0.0f, 0.0f}, 
               Vector3 scl = {1.0f, 1.0f, 1.0f},
               Color col = RED,
               bool enablePhysics = false);
    ~GameObject();
    
    // Getters
    Vector3 GetPosition() const;
    Vector3 GetRotation() const { return rotation; }
    Vector3 GetScale() const { return scale; }
    Color GetColor() const { return color; }
    PhysicsBody* GetPhysicsBody() const { return physicsBody; }
    Collider* GetCollider() const { return collider; }
    bool HasPhysics() const { return hasPhysics; }
    
    // Setters
    void SetPosition(Vector3 pos);
    void SetRotation(Vector3 rot) { rotation = rot; }
    void SetScale(Vector3 scl) { scale = scl; }
    void SetColor(Color col) { color = col; }
    
    // Transform methods
    void Move(Vector3 offset);
    void Rotate(Vector3 rotationOffset);
    void Scale(Vector3 scaleOffset);
    
    // Physics methods
    void EnablePhysics(float mass = 1.0f);
    void DisablePhysics();
    void AddForce(Vector3 force);
    void SetVelocity(Vector3 velocity);
    Vector3 GetVelocity() const;
    void Jump(float force);
    
    // Collision
    void EnableCollider(Vector3 size);
    void DisableCollider();
    void UpdateFromPhysics();
    
    // Rendering
    void Draw() const;
    void DrawWireframe() const;
};
