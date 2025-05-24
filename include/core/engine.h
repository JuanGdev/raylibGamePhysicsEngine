#pragma once
#include "raylib.h"
#include "core/GameObject.h"
#include "rendering/Renderer.h"
#include "physics/PhysicsWorld.h"
#include <vector>
#include <string>

class Engine {
private:
    int screenWidth;
    int screenHeight;
    const char* title;
    bool running;

    // Core systems
    Camera3D camera;
    Renderer renderer;
    PhysicsWorld physicsWorld;
    
    // Game objects
    GameObject cube;
    GameObject floor;
    Vector3 cameraOffset;
    
    // UI
    std::vector<std::string> uiMessages;

public:
    Engine(int width = 800, int height = 600, const char* windowTitle = "Physics Engine Project");
    ~Engine();
    
    bool Initialize();
    void Run();
    void Shutdown();
    
    bool IsRunning() const { return running; }
    
private:
    void Update();
    void Render();
    void Initialize3D();
};