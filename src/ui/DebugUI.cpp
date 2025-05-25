#include "ui/DebugUI.h"
#include <iostream>

DebugUI::DebugUI(int width, int height) 
    : debugWidth(width), debugHeight(height), debugWindowOpen(false), 
      debugWindowPosition({50.0f, 50.0f}) {
}

DebugUI::~DebugUI() {
    Shutdown();
}

void DebugUI::Initialize() {
    // The debug UI will be rendered as an overlay, not a separate window
    // Raylib doesn't support multiple windows natively, so we'll use a panel approach
    debugWindowOpen = false; // Start closed to avoid initial flickering
    std::cout << "Debug UI initialized" << std::endl;
}

void DebugUI::Update() {
    // Handle debug window controls
    if (IsKeyPressed(KEY_F1)) {
        ToggleWindow();
    }
    
    // Allow dragging the debug window
    if (debugWindowOpen && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        Vector2 mousePos = GetMousePosition();
        Rectangle titleBar = {debugWindowPosition.x, debugWindowPosition.y, (float)debugWidth, 30.0f};
        
        if (CheckCollisionPointRec(mousePos, titleBar)) {
            Vector2 mouseDelta = GetMouseDelta();
            debugWindowPosition.x += mouseDelta.x;
            debugWindowPosition.y += mouseDelta.y;
        }
    }
}

void DebugUI::Render(const GameObject& cube1, const GameObject& cube2, const std::vector<std::string>& messages) {
    if (!debugWindowOpen) return;
    
    // Draw debug window background with fully opaque colors
    Rectangle windowRect = {debugWindowPosition.x, debugWindowPosition.y, (float)debugWidth, (float)debugHeight};
    DrawRectangleRec(windowRect, (Color){30, 30, 30, 255}); // Fully opaque dark background
    DrawRectangleLinesEx(windowRect, 2.0f, LIGHTGRAY);
    
    // Draw title bar
    Rectangle titleBar = {debugWindowPosition.x, debugWindowPosition.y, (float)debugWidth, 30.0f};
    DrawRectangleRec(titleBar, (Color){50, 50, 50, 255}); // Fully opaque title bar
    DrawText("Debug Info (F1 to toggle)", (int)debugWindowPosition.x + 10, (int)debugWindowPosition.y + 8, 16, WHITE);
    
    // Draw close button
    Rectangle closeButton = {debugWindowPosition.x + debugWidth - 25, debugWindowPosition.y + 5, 20.0f, 20.0f};
    DrawRectangleRec(closeButton, (Color){180, 40, 40, 255}); // Fully opaque red
    DrawText("X", (int)closeButton.x + 6, (int)closeButton.y + 4, 12, WHITE);
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), closeButton)) {
        debugWindowOpen = false;
    }
    
    // Content area
    float contentY = debugWindowPosition.y + 40;
    float lineHeight = 18.0f;
    int textSize = 12;
    
    // Draw control instructions
    for (size_t i = 0; i < messages.size(); i++) {
        DrawText(messages[i].c_str(), (int)debugWindowPosition.x + 10, 
                (int)(contentY + i * lineHeight), textSize, LIGHTGRAY);
    }
    
    contentY += messages.size() * lineHeight + 10;
    
    // Draw physics debug info
    Vector3 cubePos = cube1.GetPosition();
    Vector3 cubeVel = cube1.GetVelocity();
    Vector3 cubeScale = cube1.GetScale();
    Vector3 cube2Pos = cube2.GetPosition();
    Vector3 cube2Vel = cube2.GetVelocity();
    Vector3 cube2Scale = cube2.GetScale();
    
    // RED Cube info
    DrawText("=== RED CUBE ===", (int)debugWindowPosition.x + 10, (int)contentY, 14, (Color){255, 100, 100, 255});
    contentY += lineHeight;
    
    DrawText(TextFormat("Pos: (%.1f, %.1f, %.1f)", cubePos.x, cubePos.y, cubePos.z), 
             (int)debugWindowPosition.x + 10, (int)contentY, textSize, WHITE);
    contentY += lineHeight;
    
    DrawText(TextFormat("Vel: (%.1f, %.1f, %.1f)", cubeVel.x, cubeVel.y, cubeVel.z), 
             (int)debugWindowPosition.x + 10, (int)contentY, textSize, WHITE);
    contentY += lineHeight;
    
    DrawText(TextFormat("Scale: (%.2f, %.2f, %.2f)", cubeScale.x, cubeScale.y, cubeScale.z), 
             (int)debugWindowPosition.x + 10, (int)contentY, textSize, WHITE);
    contentY += lineHeight;
    
    if (cube1.HasPhysics() && cube1.GetPhysicsBody()) {
        bool grounded = cube1.GetPhysicsBody()->isGrounded;
        DrawText(TextFormat("Grounded: %s", grounded ? "YES" : "NO"), 
                (int)debugWindowPosition.x + 10, (int)contentY, textSize, 
                grounded ? (Color){100, 255, 100, 255} : (Color){255, 100, 100, 255});
        contentY += lineHeight;
    }
    
    contentY += 10;
    
    // BLUE Cube info
    DrawText("=== BLUE CUBE ===", (int)debugWindowPosition.x + 10, (int)contentY, 14, (Color){100, 150, 255, 255});
    contentY += lineHeight;
    
    DrawText(TextFormat("Pos: (%.1f, %.1f, %.1f)", cube2Pos.x, cube2Pos.y, cube2Pos.z), 
             (int)debugWindowPosition.x + 10, (int)contentY, textSize, WHITE);
    contentY += lineHeight;
    
    DrawText(TextFormat("Vel: (%.1f, %.1f, %.1f)", cube2Vel.x, cube2Vel.y, cube2Vel.z), 
             (int)debugWindowPosition.x + 10, (int)contentY, textSize, WHITE);
    contentY += lineHeight;
    
    DrawText(TextFormat("Scale: (%.2f, %.2f, %.2f)", cube2Scale.x, cube2Scale.y, cube2Scale.z), 
             (int)debugWindowPosition.x + 10, (int)contentY, textSize, WHITE);
    contentY += lineHeight;
    
    if (cube2.HasPhysics() && cube2.GetPhysicsBody()) {
        bool grounded2 = cube2.GetPhysicsBody()->isGrounded;
        DrawText(TextFormat("Grounded: %s", grounded2 ? "YES" : "NO"), 
                (int)debugWindowPosition.x + 10, (int)contentY, textSize, 
                grounded2 ? (Color){100, 255, 100, 255} : (Color){100, 150, 255, 255});
        contentY += lineHeight;
    }
    
    contentY += 20;
    
    // Performance info
    DrawText("=== PERFORMANCE ===", (int)debugWindowPosition.x + 10, (int)contentY, 14, (Color){255, 255, 100, 255});
    contentY += lineHeight;
    
    DrawText(TextFormat("FPS: %d", GetFPS()), 
             (int)debugWindowPosition.x + 10, (int)contentY, textSize, WHITE);
    contentY += lineHeight;
    
    DrawText(TextFormat("Frame Time: %.2f ms", GetFrameTime() * 1000), 
             (int)debugWindowPosition.x + 10, (int)contentY, textSize, WHITE);
    contentY += lineHeight;
    
    // Instructions at the bottom
    contentY += 20;
    DrawText("F1: Toggle this window", (int)debugWindowPosition.x + 10, (int)contentY, 10, (Color){160, 160, 160, 255});
    contentY += 15;
    DrawText("Drag title bar to move", (int)debugWindowPosition.x + 10, (int)contentY, 10, (Color){160, 160, 160, 255});
}

void DebugUI::Shutdown() {
    std::cout << "Debug UI shutdown" << std::endl;
}
