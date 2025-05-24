#pragma once
#include "raylib.h"
#include "core/GameObject.h"

class Engine {
private:
    int screenWidth;
    int screenHeight;
    const char* title;
    bool running;

    Camera3D camera;
    GameObject cube;
    Vector3 cameraOffset;

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