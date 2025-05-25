#pragma once
#include "raylib.h"
#include "core/GameObject.h"
#include "rendering/Renderer.h"
#include "physics/PhysicsWorld.h"
#include "ui/DebugUI.h"
#include "ui/PhysicsUI.h"
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
    DebugUI debugUI;
    PhysicsUI physicsUI;
    
    // Game objects
    GameObject cube;        // Red cube - player controlled
    std::vector<GameObject> otherCubes;  // Other cubes - physics only
    GameObject floor;
    Vector3 cameraOffset;
    
    // UI
    std::vector<std::string> uiMessages;

public:
    Engine(int width = 1920, int height = 1080, const char* windowTitle = "Physics Engine Project");
    ~Engine();
    
    bool Initialize();
    void Run();
    void Shutdown();
    
    bool IsRunning() const { return running; }
    
private:
    void Update();
    void Render();
    void Initialize3D();
    void SpawnNewCube();
    void ResolveCubeToCubeCollision(GameObject& cube1, GameObject& cube2);
};