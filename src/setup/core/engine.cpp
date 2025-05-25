#include "core/engine.h"
#include "raymath.h"
#include <iostream>
#include <utility>

Engine::Engine(int width, int height, const char* windowTitle) 
    : screenWidth(width), screenHeight(height), title(windowTitle), running(false),
      cube({0.0f, 5.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {2.0f, 2.0f, 2.0f}, RED, true),
      floor({0.0f, -0.05f, 0.0f}, {0.0f, 0.0f, 0.0f}, {40.0f, 0.1f, 40.0f}, LIGHTGRAY, false),
      cameraOffset({4.0f, 4.0f, 4.0f}) {
    
    // Initialize with one additional cube (the blue one)
    GameObject initialCube({4.0f, 8.0f, 2.0f}, {0.0f, 0.0f, 0.0f}, {1.5f, 1.5f, 1.5f}, BLUE, true);
    otherCubes.push_back(initialCube);
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
    
    // Setup physics for all other cubes
    for (auto& otherCube : otherCubes) {
        otherCube.EnablePhysics(0.8f);  // Slightly lighter
        Vector3 cubeScale = otherCube.GetScale();
        otherCube.EnableCollider(cubeScale);
    }
    
    // Setup floor collider - make sure it matches the visual size
    floor.EnableCollider({40.0f, 0.1f, 40.0f});
    
    // Setup UI messages
    uiMessages = {
        "Physics Engine 3D - Multiple Cubes Collision Demo",
        "RED CUBE: WASD: Move | SPACE: Jump | IJKL+UO: Rotate | ZX: Scale",
        "OTHER CUBES: Physics only - no manual control",
        "CAMERA: Q/E: Orbit | T/G: Height | C: Color | R: Reset",
        "Press N to spawn new cube | F1 to toggle debug window"
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
    
    // Jump controls (only for player cube)
    if (IsKeyPressed(KEY_SPACE)) {
        cube.Jump(8.0f); // Jump force for player cube
    }
    
    // Spawn new cube control
    if (IsKeyPressed(KEY_N)) {
        SpawnNewCube();
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

    // STEP 1: Update physics for ALL objects first
    // Note: The isGrounded state is preserved from previous frame at this point
    if (cube.HasPhysics()) {
        physicsWorld.UpdatePhysicsBody(*cube.GetPhysicsBody());
        cube.UpdateFromPhysics();
    }
    
    for (auto& otherCube : otherCubes) {
        if (otherCube.HasPhysics()) {
            physicsWorld.UpdatePhysicsBody(*otherCube.GetPhysicsBody());
            otherCube.UpdateFromPhysics();
        }
    }
    
    // STEP 2: Resolve floor collisions for all objects
    if (cube.HasPhysics() && cube.GetCollider() && floor.GetCollider()) {
        if (physicsWorld.CheckCollision(*cube.GetCollider(), *floor.GetCollider())) {
            physicsWorld.ResolveCollision(*cube.GetPhysicsBody(), *floor.GetCollider());
            cube.UpdateFromPhysics();
        }
    }
    
    for (auto& otherCube : otherCubes) {
        if (otherCube.HasPhysics() && otherCube.GetCollider() && floor.GetCollider()) {
            if (physicsWorld.CheckCollision(*otherCube.GetCollider(), *floor.GetCollider())) {
                physicsWorld.ResolveCollision(*otherCube.GetPhysicsBody(), *floor.GetCollider());
                otherCube.UpdateFromPhysics();
            }
        }
    }
    
    // STEP 3: Resolve cube-to-cube collisions
    for (auto& otherCube : otherCubes) {
        if (cube.GetCollider() && otherCube.GetCollider()) {
            if (physicsWorld.CheckCollision(*cube.GetCollider(), *otherCube.GetCollider())) {
                ResolveCubeToCubeCollision(cube, otherCube);
            }
        }
    }
    
    // Check collisions between other cubes
    for (size_t i = 0; i < otherCubes.size(); i++) {
        for (size_t j = i + 1; j < otherCubes.size(); j++) {
            if (otherCubes[i].GetCollider() && otherCubes[j].GetCollider()) {
                if (physicsWorld.CheckCollision(*otherCubes[i].GetCollider(), *otherCubes[j].GetCollider())) {
                    ResolveCubeToCubeCollision(otherCubes[i], otherCubes[j]);
                }
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
        // Reset player cube (red)
        cube.SetPosition({0.0f, 5.0f, 0.0f});
        cube.SetRotation({0.0f, 0.0f, 0.0f});
        cube.SetScale({2.0f, 2.0f, 2.0f});
        cube.SetColor(RED);
        cube.SetVelocity({0.0f, 0.0f, 0.0f});
        
        // Clear all other cubes and add back the initial blue one
        otherCubes.clear();
        GameObject initialCube({4.0f, 8.0f, 2.0f}, {0.0f, 0.0f, 0.0f}, {1.5f, 1.5f, 1.5f}, BLUE, true);
        initialCube.EnablePhysics(0.8f);
        initialCube.EnableCollider(initialCube.GetScale());
        otherCubes.push_back(initialCube);
        
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
    
    // Render all other cubes
    for (const auto& otherCube : otherCubes) {
        renderer.RenderGameObject(otherCube);
    }
    
    renderer.RenderFloor(floor.GetPosition(), {40.0f, 0.1f, 40.0f}, GRAY);
    
    // Render debug colliders if available
    if (cube.GetCollider()) {
        Vector3 colliderSize = cube.GetScale(); // Use scale as collider size
        renderer.RenderCollider(cube.GetPosition(), colliderSize, GREEN);
    }
    
    // Render colliders for other cubes
    for (const auto& otherCube : otherCubes) {
        if (otherCube.GetCollider()) {
            Vector3 colliderSize = otherCube.GetScale();
            renderer.RenderCollider(otherCube.GetPosition(), colliderSize, YELLOW);
        }
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
    debugUI.Render(cube, otherCubes, uiMessages);
    
    // End the drawing frame
    EndDrawing();
}

void Engine::Shutdown() {
    if (IsWindowReady()) {
        CloseWindow();
        std::cout << "Engine shutdown complete" << std::endl;
    }
}

void Engine::SpawnNewCube() {
    // Random position around the map
    float x = GetRandomValue(-15, 15);
    float z = GetRandomValue(-15, 15);
    float y = GetRandomValue(5, 15);  // Spawn above ground
    
    // Random scale
    float scale = GetRandomValue(100, 250) / 100.0f;  // 1.0 to 2.5
    
    // Random color
    Color colors[] = {BLUE, GREEN, YELLOW, ORANGE, PURPLE, PINK, MAGENTA, LIME, GOLD, VIOLET};
    Color randomColor = colors[GetRandomValue(0, 9)];
    
    // Create new cube with physics enabled from the start to avoid copy issues
    GameObject newCube(Vector3{x, y, z}, Vector3{0.0f, 0.0f, 0.0f}, Vector3{scale, scale, scale}, randomColor, true);
    newCube.GetPhysicsBody()->mass = 0.8f;  // Set mass after creation
    newCube.EnableCollider(newCube.GetScale());
    
    // Add to vector using move semantics
    otherCubes.push_back(std::move(newCube));
    
    std::cout << "Spawned new cube at (" << x << ", " << y << ", " << z << ") with scale " << scale << std::endl;
}

void Engine::ResolveCubeToCubeCollision(GameObject& cube1, GameObject& cube2) {
    if (!cube1.HasPhysics() || !cube2.HasPhysics()) return;
    
    PhysicsBody* body1 = cube1.GetPhysicsBody();
    PhysicsBody* body2 = cube2.GetPhysicsBody();
    
    if (!body1 || !body2) return;
    
    // Usar el nuevo sistema de detección y resolución de colisiones
    physicsWorld.ResolveCubeCollision(*body1, *body2);
    
    // Actualizar las posiciones de los GameObjects desde su física
    cube1.UpdateFromPhysics();
    cube2.UpdateFromPhysics();
}