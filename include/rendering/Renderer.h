#pragma once
#include "raylib.h"
#include "core/GameObject.h"
#include <vector>
#include <string>

class Renderer {
private:
    Camera3D* camera;
    Color backgroundColor;
    bool showGrid;
    bool showWireframes;
    bool showAxisGizmos;  // Controlar la visualización de los gizmos de ejes

public:
    Renderer();
    ~Renderer();
    
    // Configuration
    void SetCamera(Camera3D* cam) { camera = cam; }
    void SetBackgroundColor(Color color) { backgroundColor = color; }
    void SetShowGrid(bool show) { showGrid = show; }
    void SetShowWireframes(bool show) { showWireframes = show; }
    void SetShowAxisGizmos(bool show) { showAxisGizmos = show; }
    
    // Rendering methods
    void BeginFrame();
    void EndFrame();
    void RenderGameObject(const GameObject& obj);
    void RenderFloor(Vector3 position, Vector3 size, Color color);
    void RenderGrid(int slices, float spacing);
    void RenderUI(const std::vector<std::string>& messages, int screenWidth, int screenHeight);
    
    // Debug rendering
    void RenderCollider(Vector3 position, Vector3 size, Color color);
};
