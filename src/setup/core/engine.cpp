#include "core/engine.h"
#include "raymath.h"
#include <iostream>

Engine::Engine(int width, int height, const char* windowTitle) 
    : screenWidth(width), screenHeight(height), title(windowTitle), running(false),
      cube({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {2.0f, 2.0f, 2.0f}, RED),
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
    // Game logic update here
    if (IsKeyPressed(KEY_ESCAPE)) {
        running = false;
    }
    
    // Cube movement controls
    Vector3 movement = {0.0f, 0.0f, 0.0f};
    float moveSpeed = 0.1f;
    
    if (IsKeyDown(KEY_W)) movement.z -= moveSpeed;
    if (IsKeyDown(KEY_S)) movement.z += moveSpeed;
    if (IsKeyDown(KEY_A)) movement.x -= moveSpeed;
    if (IsKeyDown(KEY_D)) movement.x += moveSpeed;
    if (IsKeyDown(KEY_Q)) movement.y -= moveSpeed;
    if (IsKeyDown(KEY_E)) movement.y += moveSpeed;
    
    if (Vector3Length(movement) > 0) {
        cube.Move(movement);
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
        cube.SetPosition({0.0f, 0.0f, 0.0f});
        cube.SetRotation({0.0f, 0.0f, 0.0f});
        cube.SetScale({2.0f, 2.0f, 2.0f});
        cube.SetColor(RED);
        cameraOffset = {4.0f, 4.0f, 4.0f}; // Reset camera offset
        Initialize3D();
    }
}

void Engine::Render() {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode3D(camera);

    // Draw the GameObject cube
    cube.Draw();
    cube.DrawWireframe();

    DrawGrid(20, 1.0f);

    EndMode3D();

    // 2D UI overlay
    DrawText("Physics Engine 3D - Press ESC to exit", 10, 10, 20, DARKGRAY);
    DrawText("WASD+QE: Move | IJKL+UO: Rotate | ZX: Scale | C: Color | R: Reset", 10, 30, 16, GRAY);
    DrawText("LEFT/RIGHT: Orbit camera | UP/DOWN: Camera height", 10, 50, 16, GRAY);
    DrawFPS(10, screenHeight - 30);

    EndDrawing();
}

void Engine::Shutdown() {
    if (IsWindowReady()) {
        CloseWindow();
        std::cout << "Engine shutdown complete" << std::endl;
    }
}