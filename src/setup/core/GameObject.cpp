#include "core/GameObject.h"
#include "raymath.h"
#include "rlgl.h"

GameObject::GameObject(Vector3 pos, Vector3 rot, Vector3 scl, Color col)
    : position(pos), rotation(rot), scale(scl), color(col) {
}

void GameObject::Move(Vector3 offset) {
    position = Vector3Add(position, offset);
}

void GameObject::Rotate(Vector3 rotationOffset) {
    rotation = Vector3Add(rotation, rotationOffset);
}

void GameObject::Scale(Vector3 scaleOffset) {
    scale = Vector3Add(scale, scaleOffset);
}

void GameObject::Draw() {
    // Push matrix for transformations
    rlPushMatrix();
    
    // Apply transformations
    rlTranslatef(position.x, position.y, position.z);
    rlRotatef(rotation.x, 1, 0, 0);
    rlRotatef(rotation.y, 0, 1, 0);
    rlRotatef(rotation.z, 0, 0, 1);
    rlScalef(scale.x, scale.y, scale.z);
    
    // Draw cube at origin (transformations already applied)
    DrawCube({0, 0, 0}, 1.0f, 1.0f, 1.0f, color);
    
    rlPopMatrix();
}

void GameObject::DrawWireframe() {
    rlPushMatrix();
    
    rlTranslatef(position.x, position.y, position.z);
    rlRotatef(rotation.x, 1, 0, 0);
    rlRotatef(rotation.y, 0, 1, 0);
    rlRotatef(rotation.z, 0, 0, 1);
    rlScalef(scale.x, scale.y, scale.z);
    
    DrawCubeWires({0, 0, 0}, 1.0f, 1.0f, 1.0f, MAROON);
    
    rlPopMatrix();
}
