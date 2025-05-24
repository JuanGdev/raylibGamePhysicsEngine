#pragma once
#include "raylib.h"

class GameObject {
private:
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;
    Color color;
    
public:
    GameObject(Vector3 pos = {0.0f, 0.0f, 0.0f}, 
               Vector3 rot = {0.0f, 0.0f, 0.0f}, 
               Vector3 scl = {1.0f, 1.0f, 1.0f},
               Color col = RED);
    
    // Getters
    Vector3 GetPosition() const { return position; }
    Vector3 GetRotation() const { return rotation; }
    Vector3 GetScale() const { return scale; }
    Color GetColor() const { return color; }
    
    // Setters
    void SetPosition(Vector3 pos) { position = pos; }
    void SetRotation(Vector3 rot) { rotation = rot; }
    void SetScale(Vector3 scl) { scale = scl; }
    void SetColor(Color col) { color = col; }
    
    // Transform methods
    void Move(Vector3 offset);
    void Rotate(Vector3 rotationOffset);
    void Scale(Vector3 scaleOffset);
    
    // Rendering
    void Draw();
    void DrawWireframe();
};
