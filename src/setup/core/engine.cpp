#include "core/engine.h"
#include "raymath.h"
#include <iostream>

Engine::Engine(int width, int height, const char* windowTitle) 
    : screenWidth(width), screenHeight(height), title(windowTitle), running(false),
      cube({0.0f, 5.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {2.0f, 2.0f, 2.0f}, RED, true),
      cube2({4.0f, 8.0f, 2.0f}, {0.0f, 0.0f, 0.0f}, {1.5f, 1.5f, 1.5f}, BLUE, true),
      floor({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {40.0f, 2.0f, 40.0f}, LIGHTGRAY, false),
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
    
    // Setup physics for second cube
    cube2.EnablePhysics(0.8f);  // Slightly lighter
    cube2.EnableCollider({1.5f, 1.5f, 1.5f});
    
    // Setup floor collider
    floor.EnableCollider({40.0f, 2.0f, 40.0f});
    
    // Setup UI messages
    uiMessages = {
        "Physics Engine 3D - Two Cubes Collision Demo",
        "RED CUBE: WASD: Move | SPACE: Jump | IJKL+UO: Rotate | ZX: Scale",
        "BLUE CUBE: Arrow Keys: Move | ENTER: Jump",
        "CAMERA: Q/E: Orbit | T/G: Height | C: Color | R: Reset",
        "Press F1 to toggle debug window"
    };
    
    // Initialize debug UI
    debugUI.Initialize();
    
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
    
    // Update debug UI first
    debugUI.Update();
    
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
    
    // Second cube movement controls (Arrow keys)
    Vector3 movement2 = {0.0f, 0.0f, 0.0f};
    
    if (IsKeyDown(KEY_UP)) movement2.z -= moveSpeed;
    if (IsKeyDown(KEY_DOWN)) movement2.z += moveSpeed;
    if (IsKeyDown(KEY_LEFT)) movement2.x -= moveSpeed;
    if (IsKeyDown(KEY_RIGHT)) movement2.x += moveSpeed;
    
    if (Vector3Length(movement2) > 0) {
        cube2.AddForce(movement2);
    }
    
    // Jump controls
    if (IsKeyPressed(KEY_SPACE)) {
        cube.Jump(8.0f); // Jump force for cube 1
    }
    if (IsKeyPressed(KEY_ENTER)) {
        cube2.Jump(8.0f); // Jump force for cube 2
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

    // Update physics for both cubes
    if (cube.HasPhysics()) {
        // First update physics (applies gravity and movement)
        physicsWorld.UpdatePhysicsBody(*cube.GetPhysicsBody());
        cube.UpdateFromPhysics();
        
        // Check collision with floor
        if (cube.GetCollider() && floor.GetCollider()) {
            if (physicsWorld.CheckCollision(*cube.GetCollider(), *floor.GetCollider())) {
                physicsWorld.ResolveCollision(*cube.GetPhysicsBody(), *floor.GetCollider());
                cube.UpdateFromPhysics();
            }
        }
    }
    
    if (cube2.HasPhysics()) {
        // Update physics for second cube
        physicsWorld.UpdatePhysicsBody(*cube2.GetPhysicsBody());
        cube2.UpdateFromPhysics();
        
        // Check collision with floor
        if (cube2.GetCollider() && floor.GetCollider()) {
            if (physicsWorld.CheckCollision(*cube2.GetCollider(), *floor.GetCollider())) {
                physicsWorld.ResolveCollision(*cube2.GetPhysicsBody(), *floor.GetCollider());
                cube2.UpdateFromPhysics();
            }
        }
        
        // Check collision between cubes
        if (cube.GetCollider() && cube2.GetCollider()) {
            if (physicsWorld.CheckCollision(*cube.GetCollider(), *cube2.GetCollider())) {
                // For cube-to-cube collision, we need a more sophisticated approach
                // For now, let's implement a simple elastic collision
                // ResolveCubeToCubeCollision(cube, cube2); // TODO: Implement this function
            }
        }
    }
    
    // Update camera to follow cube
    Vector3 cubePos = cube.GetPosition();
    
    camera.target = cubePos; // La cámara siempre mira al cubo
    camera.position = Vector3Add(cubePos, cameraOffset); // Posición relativa al cubo
    
    // Camera controls - orbitar alrededor del cubo
    if (IsKeyDown(KEY_Q)) {
        // Rotar cameraOffset alrededor del eje Y (izquierda)
        float angle = -0.05f;
        float newX = cameraOffset.x * cosf(angle) - cameraOffset.z * sinf(angle);
        float newZ = cameraOffset.x * sinf(angle) + cameraOffset.z * cosf(angle);
        cameraOffset.x = newX;
        cameraOffset.z = newZ;
        camera.position = Vector3Add(cubePos, cameraOffset);
    }
    if (IsKeyDown(KEY_E)) {
        // Rotar cameraOffset alrededor del eje Y (derecha)
        float angle = 0.05f;
        float newX = cameraOffset.x * cosf(angle) - cameraOffset.z * sinf(angle);
        float newZ = cameraOffset.x * sinf(angle) + cameraOffset.z * cosf(angle);
        cameraOffset.x = newX;
        cameraOffset.z = newZ;
        camera.position = Vector3Add(cubePos, cameraOffset);
    }
    
    // Camera vertical controls
    if (IsKeyDown(KEY_T)) {
        cameraOffset.y += 0.1f; // Mover cámara hacia arriba
        camera.position = Vector3Add(cubePos, cameraOffset);
    }
    if (IsKeyDown(KEY_G)) {
        cameraOffset.y -= 0.1f; // Mover cámara hacia abajo
        camera.position = Vector3Add(cubePos, cameraOffset);
    }

    // Reset cubes
    if (IsKeyPressed(KEY_R)) {
        // Reset first cube (red)
        cube.SetPosition({0.0f, 5.0f, 0.0f});
        cube.SetRotation({0.0f, 0.0f, 0.0f});
        cube.SetScale({2.0f, 2.0f, 2.0f});
        cube.SetColor(RED);
        cube.SetVelocity({0.0f, 0.0f, 0.0f});
        
        // Reset second cube (blue)
        cube2.SetPosition({4.0f, 8.0f, 2.0f});
        cube2.SetRotation({0.0f, 0.0f, 0.0f});
        cube2.SetScale({1.5f, 1.5f, 1.5f});
        cube2.SetColor(BLUE);
        cube2.SetVelocity({0.0f, 0.0f, 0.0f});
        
        cameraOffset = {4.0f, 4.0f, 4.0f}; // Reset camera offset
        Initialize3D();
    }
}

void Engine::Render() {
    // Start drawing frame
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    // 3D rendering
    BeginMode3D(camera);
    
    // Render game objects
    renderer.RenderGameObject(cube);
    renderer.RenderGameObject(cube2);
    renderer.RenderFloor(floor.GetPosition(), {40.0f, 0.1f, 40.0f}, GRAY);
    
    // Render debug colliders if available
    if (cube.GetCollider()) {
        Vector3 colliderSize = cube.GetScale(); // Use scale as collider size
        renderer.RenderCollider(cube.GetPosition(), colliderSize, GREEN);
    }
    if (cube2.GetCollider()) {
        Vector3 colliderSize2 = cube2.GetScale(); // Use scale as collider size
        renderer.RenderCollider(cube2.GetPosition(), colliderSize2, YELLOW);
    }
    if (floor.GetCollider()) {
        Vector3 floorColliderSize = {40.0f, 0.1f, 40.0f}; // Floor size
        renderer.RenderCollider(floor.GetPosition(), floorColliderSize, BLUE);
    }
    
    // Render grid aligned with floor (40x40 grid with 1.0f spacing)
    renderer.RenderGrid(40, 1.0f);
    
    // End 3D mode
    EndMode3D();
    
    // 2D UI overlay (rendered after 3D content but within the same drawing frame)
    DrawText("Physics Engine 3D", 10, 10, 20, DARKGRAY);
    DrawText("Press F1 for debug info | ESC to exit", 10, 35, 14, GRAY);
    
    // Render debug UI (also 2D overlay)
    debugUI.Render(cube, cube2, uiMessages);
    
    // End the drawing frame
    EndDrawing();
}

void Engine::Shutdown() {
    if (IsWindowReady()) {
        CloseWindow();
        std::cout << "Engine shutdown complete" << std::endl;
    }
}