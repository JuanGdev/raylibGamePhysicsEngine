#include "core/engine.h"
#include "raymath.h"
#include <iostream>

Engine::Engine(int width, int height, const char* windowTitle) 
    : screenWidth(width), screenHeight(height), title(windowTitle), running(false),
      cube({0.0f, 5.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {2.0f, 2.0f, 2.0f}, RED, true),
      floor({0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {20.0f, 2.0f, 20.0f}, LIGHTGRAY, false),
      cameraOffset({4.0f, 4.0f, 4.0f}) {
}

Engine::~Engine() {
    Shutdown();
}

bool Engine::Initialize() {
    InitWindow(screenWidth, screenHeight, title);
    SetTargetFPS(60);
    
    if (!IsWindowReady()) {
        std::cerr << "Failed to initialize window" << std::endl;
        return false;
    }

    Initialize3D();
    
    // Setup renderer
    renderer.SetCamera(&camera);
    renderer.SetBackgroundColor(RAYWHITE);
    
    // Setup physics for cube
    cube.EnablePhysics(1.0f);
    cube.EnableCollider({2.0f, 2.0f, 2.0f});
    
    // Setup floor collider
    floor.EnableCollider({20.0f, 2.0f, 20.0f});
    
    // Setup UI messages
    uiMessages = {
        "Physics Engine 3D - Press ESC to exit",
        "WASD: Move | SPACE: Jump | C: Color | R: Reset",
        "LEFT/RIGHT: Orbit | UP/DOWN: Camera Height"
    };
    
    running = true;
    std::cout << "Engine initialized successfully" << std::endl;
    return true;
}

void Engine::Initialize3D(){
    camera.position = (Vector3){4.0f, 4.0f, 4.0f};
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
}

void Engine::Run() {
    while (running && !WindowShouldClose()) {
        Update();
        Render();
    }
}

void Engine::Update() {
    float deltaTime = GetFrameTime();
    
    // Game logic update here
    if (IsKeyPressed(KEY_ESCAPE)) {
        running = false;
    }
    
    // Update physics world
    physicsWorld.Update(deltaTime);
    
    // Cube movement controls (horizontal only, gravity handles vertical)
    Vector3 movement = {0.0f, 0.0f, 0.0f};
    float moveSpeed = 5.0f; // Force instead of direct movement
    
    if (IsKeyDown(KEY_W)) movement.z -= moveSpeed;
    if (IsKeyDown(KEY_S)) movement.z += moveSpeed;
    if (IsKeyDown(KEY_A)) movement.x -= moveSpeed;
    if (IsKeyDown(KEY_D)) movement.x += moveSpeed;
    
    if (Vector3Length(movement) > 0) {
        cube.AddForce(movement);
    }
    
    // Jump control
    if (IsKeyPressed(KEY_SPACE)) {
        cube.Jump(8.0f); // Jump force
    }
    
    // Cube rotation controls
    Vector3 rotationSpeed = {2.0f, 2.0f, 2.0f};
    if (IsKeyDown(KEY_I)) cube.Rotate({-rotationSpeed.x, 0.0f, 0.0f});
    if (IsKeyDown(KEY_K)) cube.Rotate({rotationSpeed.x, 0.0f, 0.0f});
    if (IsKeyDown(KEY_J)) cube.Rotate({0.0f, -rotationSpeed.y, 0.0f});
    if (IsKeyDown(KEY_L)) cube.Rotate({0.0f, rotationSpeed.y, 0.0f});
    if (IsKeyDown(KEY_U)) cube.Rotate({0.0f, 0.0f, -rotationSpeed.z});
    if (IsKeyDown(KEY_O)) cube.Rotate({0.0f, 0.0f, rotationSpeed.z});
    
    // Cube scaling controls
    if (IsKeyDown(KEY_Z)) cube.Scale({0.01f, 0.01f, 0.01f});
    if (IsKeyDown(KEY_X)) cube.Scale({-0.01f, -0.01f, -0.01f});
    
    // Color change
    if (IsKeyPressed(KEY_C)) {
        Color colors[] = {RED, GREEN, BLUE, YELLOW, ORANGE, PURPLE, PINK};
        int colorIndex = GetRandomValue(0, 6);
        cube.SetColor(colors[colorIndex]);
    }

    // Update physics for cube
    if (cube.HasPhysics()) {
        // First update physics (applies gravity and movement)
        physicsWorld.UpdatePhysicsBody(*cube.GetPhysicsBody());
        
        // Update object position from physics
        cube.UpdateFromPhysics();
        
        // Then check collision with floor after position update
        if (cube.GetCollider() && floor.GetCollider()) {
            if (physicsWorld.CheckCollision(*cube.GetCollider(), *floor.GetCollider())) {
                physicsWorld.ResolveCollision(*cube.GetPhysicsBody(), *floor.GetCollider());
                // Update position again after collision resolution
                cube.UpdateFromPhysics();
            }
        }
    }
    
    // Update camera to follow cube
    Vector3 cubePos = cube.GetPosition();
    
    camera.target = cubePos; // La cámara siempre mira al cubo
    camera.position = Vector3Add(cubePos, cameraOffset); // Posición relativa al cubo
    
    // Camera controls - opcional para orbitar alrededor del cubo
    if (IsKeyDown(KEY_LEFT)) {
        // Rotar cameraOffset alrededor del eje Y
        float angle = -0.05f;
        float newX = cameraOffset.x * cosf(angle) - cameraOffset.z * sinf(angle);
        float newZ = cameraOffset.x * sinf(angle) + cameraOffset.z * cosf(angle);
        cameraOffset.x = newX;
        cameraOffset.z = newZ;
        camera.position = Vector3Add(cubePos, cameraOffset);
    }
    if (IsKeyDown(KEY_RIGHT)) {
        // Rotar cameraOffset alrededor del eje Y
        float angle = 0.05f;
        float newX = cameraOffset.x * cosf(angle) - cameraOffset.z * sinf(angle);
        float newZ = cameraOffset.x * sinf(angle) + cameraOffset.z * cosf(angle);
        cameraOffset.x = newX;
        cameraOffset.z = newZ;
        camera.position = Vector3Add(cubePos, cameraOffset);
    }
    
    // Camera vertical controls
    if (IsKeyDown(KEY_UP)) {
        cameraOffset.y += 0.1f; // Mover cámara hacia arriba
        camera.position = Vector3Add(cubePos, cameraOffset);
    }
    if (IsKeyDown(KEY_DOWN)) {
        cameraOffset.y -= 0.1f; // Mover cámara hacia abajo
        camera.position = Vector3Add(cubePos, cameraOffset);
    }

    // Reset cube
    if (IsKeyPressed(KEY_R)) {
        cube.SetPosition({0.0f, 5.0f, 0.0f});
        cube.SetRotation({0.0f, 0.0f, 0.0f});
        cube.SetScale({2.0f, 2.0f, 2.0f});
        cube.SetColor(RED);
        cube.SetVelocity({0.0f, 0.0f, 0.0f});
        cameraOffset = {4.0f, 4.0f, 4.0f}; // Reset camera offset
        Initialize3D();
    }
}

void Engine::Render() {
    // Use the new Renderer module
    renderer.BeginFrame();
    
    // Render game objects
    renderer.RenderGameObject(cube);
    renderer.RenderFloor(floor.GetPosition(), {20.0f, 0.1f, 20.0f}, GRAY);
    
    // Render debug colliders if available
    if (cube.GetCollider()) {
        Vector3 colliderSize = {2.0f, 2.0f, 2.0f}; // Cube size
        renderer.RenderCollider(cube.GetPosition(), colliderSize, GREEN);
    }
    if (floor.GetCollider()) {
        Vector3 floorColliderSize = {20.0f, 0.1f, 20.0f}; // Floor size
        renderer.RenderCollider(floor.GetPosition(), floorColliderSize, BLUE);
    }
    
    // Render grid
    renderer.RenderGrid(20, 1.0f);
    
    renderer.EndFrame();

    // 2D UI overlay (drawn after EndFrame to be on top)
    DrawText("Physics Engine 3D - Press ESC to exit", 10, 10, 20, DARKGRAY);
    DrawText("WASD: Force Movement | SPACE: Jump | IJKL+UO: Rotate | ZX: Scale", 10, 30, 16, GRAY);
    DrawText("LEFT/RIGHT: Orbit camera | UP/DOWN: Camera height | C: Color | R: Reset", 10, 50, 16, GRAY);
    
    // Physics debug info
    Vector3 cubePos = cube.GetPosition();
    Vector3 cubeVel = cube.GetVelocity();
    DrawText(TextFormat("Position: (%.2f, %.2f, %.2f)", cubePos.x, cubePos.y, cubePos.z), 10, 80, 16, DARKGREEN);
    DrawText(TextFormat("Velocity: (%.2f, %.2f, %.2f)", cubeVel.x, cubeVel.y, cubeVel.z), 10, 100, 16, DARKGREEN);
    
    if (cube.HasPhysics() && cube.GetPhysicsBody()) {
        bool grounded = cube.GetPhysicsBody()->isGrounded;
        DrawText(TextFormat("Grounded: %s", grounded ? "YES" : "NO"), 10, 120, 16, grounded ? GREEN : RED);
    }
    
    DrawFPS(10, screenHeight - 30);
}

void Engine::Shutdown() {
    if (IsWindowReady()) {
        CloseWindow();
        std::cout << "Engine shutdown complete" << std::endl;
    }
}