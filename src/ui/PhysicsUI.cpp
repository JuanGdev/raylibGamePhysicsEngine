#include "ui/PhysicsUI.h"
#include "raymath.h"
#include <iostream>
#include <string>

PhysicsUI::PhysicsUI(int width, int height) 
    : windowWidth(width), windowHeight(height), windowOpen(false) {
    
    // Posicionarlo en la esquina superior derecha
    windowPosition.x = GetScreenWidth() - windowWidth - 20;  // 20px de margen
    windowPosition.y = 20;  // 20px de margen superior
    
    // Inicializar los parámetros con valores predeterminados
    params.gravityMagnitude = 9.81f;
    params.gravityDirection = Vector3Normalize((Vector3){0.0f, -1.0f, 0.0f});
    params.restitution = 0.3f;
    params.friction = 0.92f;
    params.airResistance = 0.98f;
    params.launchVelocity = 10.0f;
    params.launchAngle = 45.0f;
    params.groundedStability = 3;
    params.velocityThreshold = 0.005f;
}

PhysicsUI::~PhysicsUI() {
    // Nada que limpiar por ahora
}

void PhysicsUI::Initialize() {
    windowOpen = false;  // Comenzar cerrado
    std::cout << "Physics UI initialized" << std::endl;
}

void PhysicsUI::Update(PhysicsWorld& physicsWorld) {
    // Controlar apertura/cierre de la ventana
    if (IsKeyPressed(KEY_F2)) {
        ToggleWindow();
    }
    
    // No permitir mover la ventana - siempre fija en la esquina superior derecha
    windowPosition.x = GetScreenWidth() - windowWidth - 20;
}

void PhysicsUI::Render() {
    if (!windowOpen) return;
    
    // Dibujar el fondo de la ventana
    Rectangle windowRect = {windowPosition.x, windowPosition.y, (float)windowWidth, (float)windowHeight};
    DrawRectangleRec(windowRect, (Color){30, 30, 30, 230});  // Fondo semitransparente
    DrawRectangleLinesEx(windowRect, 2.0f, SKYBLUE);
    
    // Dibujar la barra de título
    Rectangle titleBar = {windowPosition.x, windowPosition.y, (float)windowWidth, 30.0f};
    DrawRectangleRec(titleBar, (Color){0, 128, 255, 255});
    DrawText("Physics Parameters (F2)", (int)windowPosition.x + 10, (int)windowPosition.y + 8, 16, WHITE);
    
    // Posición inicial para el contenido
    float yPos = windowPosition.y + 40;
    float xPos = windowPosition.x + 10;
    float ySpacing = 40;  // Espacio entre controles
    float sliderWidth = windowWidth - 30;
    
    // Mostrar y permitir ajustar los parámetros
    // Gravedad
    DrawText("Gravity:", xPos, yPos, 14, WHITE);
    DrawText(TextFormat("%.2f m/s²", params.gravityMagnitude), xPos + 140, yPos, 14, YELLOW);
    yPos += 20;
    
    // Slider para la magnitud de la gravedad (0-20)
    DrawText("Magnitude", xPos, yPos, 14, LIGHTGRAY);
    DrawRectangleRec((Rectangle){xPos + 80, yPos, sliderWidth - 80, 10}, DARKGRAY);
    DrawRectangleRec((Rectangle){xPos + 80, yPos, (params.gravityMagnitude / 20.0f) * (sliderWidth - 80), 10}, SKYBLUE);
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && 
        CheckCollisionPointRec(GetMousePosition(), 
                               (Rectangle){xPos + 80, yPos - 5, sliderWidth - 80, 20})) {
        params.gravityMagnitude = 20.0f * (GetMousePosition().x - (xPos + 80)) / (sliderWidth - 80);
        params.gravityMagnitude = fmax(0.0f, fmin(20.0f, params.gravityMagnitude));
    }
    yPos += ySpacing;
    
    // Restitución (rebote)
    DrawText("Restitution:", xPos, yPos, 14, WHITE);
    DrawText(TextFormat("%.2f", params.restitution), xPos + 140, yPos, 14, YELLOW);
    yPos += 20;
    
    // Slider para la restitución (0-1)
    DrawText("Bounce", xPos, yPos, 14, LIGHTGRAY);
    DrawRectangleRec((Rectangle){xPos + 80, yPos, sliderWidth - 80, 10}, DARKGRAY);
    DrawRectangleRec((Rectangle){xPos + 80, yPos, params.restitution * (sliderWidth - 80), 10}, SKYBLUE);
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && 
        CheckCollisionPointRec(GetMousePosition(), 
                               (Rectangle){xPos + 80, yPos - 5, sliderWidth - 80, 20})) {
        params.restitution = (GetMousePosition().x - (xPos + 80)) / (sliderWidth - 80);
        params.restitution = fmax(0.0f, fmin(1.0f, params.restitution));
    }
    yPos += ySpacing;
    
    // Fricción
    DrawText("Friction:", xPos, yPos, 14, WHITE);
    DrawText(TextFormat("%.2f", params.friction), xPos + 140, yPos, 14, YELLOW);
    yPos += 20;
    
    // Slider para la fricción (0-1)
    DrawText("Surface", xPos, yPos, 14, LIGHTGRAY);
    DrawRectangleRec((Rectangle){xPos + 80, yPos, sliderWidth - 80, 10}, DARKGRAY);
    DrawRectangleRec((Rectangle){xPos + 80, yPos, params.friction * (sliderWidth - 80), 10}, SKYBLUE);
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && 
        CheckCollisionPointRec(GetMousePosition(), 
                               (Rectangle){xPos + 80, yPos - 5, sliderWidth - 80, 20})) {
        params.friction = (GetMousePosition().x - (xPos + 80)) / (sliderWidth - 80);
        params.friction = fmax(0.0f, fmin(1.0f, params.friction));
    }
    yPos += ySpacing;
    
    // Resistencia del aire
    DrawText("Air Resistance:", xPos, yPos, 14, WHITE);
    DrawText(TextFormat("%.2f", params.airResistance), xPos + 140, yPos, 14, YELLOW);
    yPos += 20;
    
    // Slider para la resistencia del aire (0.8-1)
    DrawText("Damping", xPos, yPos, 14, LIGHTGRAY);
    DrawRectangleRec((Rectangle){xPos + 80, yPos, sliderWidth - 80, 10}, DARKGRAY);
    DrawRectangleRec((Rectangle){xPos + 80, yPos, (params.airResistance - 0.8f) * 5 * (sliderWidth - 80), 10}, SKYBLUE);
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && 
        CheckCollisionPointRec(GetMousePosition(), 
                               (Rectangle){xPos + 80, yPos - 5, sliderWidth - 80, 20})) {
        params.airResistance = 0.8f + ((GetMousePosition().x - (xPos + 80)) / (sliderWidth - 80)) * 0.2f;
        params.airResistance = fmax(0.8f, fmin(1.0f, params.airResistance));
    }
    yPos += ySpacing;
    
    // Lanzamiento (tiro parabólico)
    DrawText("Launch Velocity:", xPos, yPos, 14, WHITE);
    DrawText(TextFormat("%.2f m/s", params.launchVelocity), xPos + 140, yPos, 14, YELLOW);
    yPos += 20;
    
    // Slider para la velocidad de lanzamiento (0-20)
    DrawText("Speed", xPos, yPos, 14, LIGHTGRAY);
    DrawRectangleRec((Rectangle){xPos + 80, yPos, sliderWidth - 80, 10}, DARKGRAY);
    DrawRectangleRec((Rectangle){xPos + 80, yPos, (params.launchVelocity / 20.0f) * (sliderWidth - 80), 10}, SKYBLUE);
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && 
        CheckCollisionPointRec(GetMousePosition(), 
                               (Rectangle){xPos + 80, yPos - 5, sliderWidth - 80, 20})) {
        params.launchVelocity = 20.0f * (GetMousePosition().x - (xPos + 80)) / (sliderWidth - 80);
        params.launchVelocity = fmax(0.0f, fmin(20.0f, params.launchVelocity));
    }
    yPos += ySpacing;
    
    // Ángulo de lanzamiento
    DrawText("Launch Angle:", xPos, yPos, 14, WHITE);
    DrawText(TextFormat("%.1f deg", params.launchAngle), xPos + 140, yPos, 14, YELLOW);
    yPos += 20;
    
    // Slider para el ángulo de lanzamiento (0-90 grados)
    DrawText("Angle", xPos, yPos, 14, LIGHTGRAY);
    DrawRectangleRec((Rectangle){xPos + 80, yPos, sliderWidth - 80, 10}, DARKGRAY);
    DrawRectangleRec((Rectangle){xPos + 80, yPos, (params.launchAngle / 90.0f) * (sliderWidth - 80), 10}, SKYBLUE);
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && 
        CheckCollisionPointRec(GetMousePosition(), 
                               (Rectangle){xPos + 80, yPos - 5, sliderWidth - 80, 20})) {
        params.launchAngle = 90.0f * (GetMousePosition().x - (xPos + 80)) / (sliderWidth - 80);
        params.launchAngle = fmax(0.0f, fmin(90.0f, params.launchAngle));
    }
    yPos += ySpacing;
    
    // Dibujar un pequeño diagrama de tiro parabólico
    yPos += 20;
    DrawText("Parabolic Trajectory Preview:", xPos, yPos, 14, WHITE);
    yPos += 20;
    
    // Punto de origen
    float originX = xPos + 20;
    float originY = yPos + 80;
    float scale = 5.0f;  // Escala del dibujo
    
    // Dibujar suelo
    DrawLine(originX - 10, originY, originX + 160, originY, WHITE);
    
    // Calcular puntos para la trayectoria parabólica
    float angleRad = params.launchAngle * DEG2RAD;
    float vx = params.launchVelocity * cosf(angleRad);
    float vy = params.launchVelocity * sinf(angleRad);
    float g = params.gravityMagnitude;
    
    // Dibujar trayectoria con puntos
    float prevX = originX;
    float prevY = originY;
    
    for (float t = 0.0f; t <= 3.0f; t += 0.1f) {
        float x = originX + (vx * t) * scale;
        float y = originY - (vy * t - 0.5f * g * t * t) * scale; // Invertir Y para la pantalla
        
        // Si la trayectoria toca el suelo, detenerse
        if (y > originY) break;
        
        // Dibujar línea entre puntos
        DrawLine(prevX, prevY, x, y, RED);
        prevX = x;
        prevY = y;
    }
    
    // Botón para probar el lanzamiento
    yPos += 100;
    Rectangle launchButton = {xPos + 10, yPos, sliderWidth - 20, 30};
    DrawRectangleRec(launchButton, (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), launchButton)) ? DARKBLUE : BLUE);
    DrawText("TEST LAUNCH (P)", xPos + 50, yPos + 7, 16, WHITE);
    
    // Instrucciones
    yPos += 50;
    DrawText("Press P to test launch with white cube", xPos + 10, yPos, 14, GREEN);
}

void PhysicsUI::ApplyParameters(PhysicsWorld& physicsWorld) {
    // Aplicar los parámetros al mundo físico
    
    // Gravedad
    physicsWorld.SetGravity(Vector3Scale(params.gravityDirection, params.gravityMagnitude));
    
    // Parámetros de colisión 
    physicsWorld.SetRestitution(params.restitution);         // Coeficiente de rebote
    physicsWorld.SetFriction(params.friction);              // Fricción de superficie
    physicsWorld.SetAirResistance(params.airResistance);    // Resistencia del aire
    
    // Parámetros de estabilidad
    physicsWorld.SetGroundedStability(params.groundedStability); // Frames para estabilización
    physicsWorld.SetVelocityThreshold(params.velocityThreshold); // Umbral de velocidad
    
    // Nota: Los parámetros de lanzamiento no se aplican directamente al mundo físico,
    // sino que se usarán cuando el usuario solicite un lanzamiento
}
