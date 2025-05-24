#include "core/GameObject.h"
#include "raymath.h"
#include "rlgl.h"

GameObject::GameObject(Vector3 pos, Vector3 rot, Vector3 scl, Color col, bool enablePhysics)
    : position(pos), rotation(rot), scale(scl), color(col), 
      physicsBody(nullptr), collider(nullptr), hasPhysics(false) {
    if (enablePhysics) {
        EnablePhysics();
    }
}

GameObject::~GameObject() {
    DisablePhysics();
    DisableCollider();
}

Vector3 GameObject::GetPosition() const {
    if (hasPhysics && physicsBody) {
        return physicsBody->position;
    }
    return position;
}

void GameObject::SetPosition(Vector3 pos) {
    position = pos;
    if (hasPhysics && physicsBody) {
        physicsBody->position = pos;
    }
    if (collider) {
        collider->position = pos;
    }
}

void GameObject::Move(Vector3 offset) {
    Vector3 newPos = Vector3Add(GetPosition(), offset);
    SetPosition(newPos);
}

void GameObject::Rotate(Vector3 rotationOffset) {
    rotation = Vector3Add(rotation, rotationOffset);
}

void GameObject::Scale(Vector3 scaleOffset) {
    scale = Vector3Add(scale, scaleOffset);
}

void GameObject::EnablePhysics(float mass) {
    if (!hasPhysics) {
        physicsBody = new PhysicsBody(position, mass);
        hasPhysics = true;
    }
}

void GameObject::DisablePhysics() {
    if (hasPhysics && physicsBody) {
        delete physicsBody;
        physicsBody = nullptr;
        hasPhysics = false;
    }
}

void GameObject::AddForce(Vector3 force) {
    if (hasPhysics && physicsBody) {
        Vector3 acceleration = Vector3Scale(force, 1.0f / physicsBody->mass);
        physicsBody->acceleration = Vector3Add(physicsBody->acceleration, acceleration);
    }
}

void GameObject::SetVelocity(Vector3 velocity) {
    if (hasPhysics && physicsBody) {
        physicsBody->velocity = velocity;
    }
}

Vector3 GameObject::GetVelocity() const {
    if (hasPhysics && physicsBody) {
        return physicsBody->velocity;
    }
    return {0.0f, 0.0f, 0.0f};
}

void GameObject::Jump(float force) {
    if (hasPhysics && physicsBody && physicsBody->isGrounded) {
        physicsBody->velocity.y = force;
        physicsBody->isGrounded = false;
    }
}

void GameObject::EnableCollider(Vector3 size) {
    if (!collider) {
        collider = new Collider(GetPosition(), size, false);
    }
}

void GameObject::DisableCollider() {
    if (collider) {
        delete collider;
        collider = nullptr;
    }
}

void GameObject::UpdateFromPhysics() {
    if (hasPhysics && physicsBody) {
        position = physicsBody->position;
        if (collider) {
            collider->position = position;
        }
    }
}

void GameObject::Draw() const {
    Vector3 currentPos = GetPosition();
    
    // Push matrix for transformations
    rlPushMatrix();
    
    // Apply transformations
    rlTranslatef(currentPos.x, currentPos.y, currentPos.z);
    rlRotatef(rotation.x, 1, 0, 0);
    rlRotatef(rotation.y, 0, 1, 0);
    rlRotatef(rotation.z, 0, 0, 1);
    rlScalef(scale.x, scale.y, scale.z);
    
    // Draw cube at origin (transformations already applied)
    DrawCube({0, 0, 0}, 1.0f, 1.0f, 1.0f, color);
    
    rlPopMatrix();
}

void GameObject::DrawWireframe() const {
    Vector3 currentPos = GetPosition();
    
    rlPushMatrix();
    
    rlTranslatef(currentPos.x, currentPos.y, currentPos.z);
    rlRotatef(rotation.x, 1, 0, 0);
    rlRotatef(rotation.y, 0, 1, 0);
    rlRotatef(rotation.z, 0, 0, 1);
    rlScalef(scale.x, scale.y, scale.z);
    
    DrawCubeWires({0, 0, 0}, 1.0f, 1.0f, 1.0f, MAROON);
    
    rlPopMatrix();
}
