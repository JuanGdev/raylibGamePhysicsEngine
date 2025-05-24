#include "rendering/Renderer.h"
#include <iostream>

Renderer::Renderer() 
    : camera(nullptr), backgroundColor(RAYWHITE), showGrid(true), showWireframes(true) {
}

Renderer::~Renderer() {
    // Nothing to cleanup for now
}

void Renderer::BeginFrame() {
    BeginDrawing();
    ClearBackground(backgroundColor);
    
    if (camera != nullptr) {
        BeginMode3D(*camera);
    }
}

void Renderer::EndFrame() {
    if (camera != nullptr) {
        EndMode3D();
    }
    EndDrawing();
}

void Renderer::RenderGameObject(const GameObject& obj) {
    obj.Draw();
    if (showWireframes) {
        obj.DrawWireframe();
    }
}

void Renderer::RenderFloor(Vector3 position, Vector3 size, Color color) {
    DrawCube(position, size.x, size.y, size.z, color);
}

void Renderer::RenderGrid(int slices, float spacing) {
    if (showGrid) {
        DrawGrid(slices, spacing);
    }
}

void Renderer::RenderUI(const std::vector<std::string>& messages, int screenWidth, int screenHeight) {
    int yOffset = 10;
    
    for (const auto& message : messages) {
        DrawText(message.c_str(), 10, yOffset, 16, DARKGRAY);
        yOffset += 20;
    }
    
    // Draw FPS
    DrawFPS(10, screenHeight - 30);
}

void Renderer::RenderCollider(Vector3 position, Vector3 size, Color color) {
    DrawCubeWires(position, size.x, size.y, size.z, color);
}
