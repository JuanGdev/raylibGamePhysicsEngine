#pragma once
#include "raylib.h"
#include "physics/PhysicsWorld.h"
#include <vector>
#include <string>

class PhysicsUI {
private:
    int windowWidth;
    int windowHeight;
    bool windowOpen;
    Vector2 windowPosition;
    
    // Physics parameters to control
    struct PhysicsParams {
        // Gravity parameters
        float gravityMagnitude;
        Vector3 gravityDirection;
        
        // Collision parameters
        float restitution;     // Coeficiente de rebote (0.0 - 1.0)
        float friction;        // Fricción de superficie (0.0 - 1.0)
        float airResistance;   // Resistencia del aire (amortiguamiento)
        
        // Lanzamiento/Tiro parabólico
        float launchVelocity;  // Velocidad inicial de lanzamiento
        float launchAngle;     // Ángulo de lanzamiento en grados
        
        // Estabilidad física
        int groundedStability; // Frames para estabilizar estado grounded
        float velocityThreshold; // Umbral para velocidades pequeñas
    };
    
    PhysicsParams params;
    
public:
    PhysicsUI(int width = 300, int height = 700);
    ~PhysicsUI();
    
    void Initialize();
    void Update(PhysicsWorld& physicsWorld);
    void Render();
    void ApplyParameters(PhysicsWorld& physicsWorld);
    
    bool IsOpen() const { return windowOpen; }
    void SetOpen(bool open) { windowOpen = open; }
    void ToggleWindow() { windowOpen = !windowOpen; }
    
    // Getters para parámetros
    float GetRestitution() const { return params.restitution; }
    float GetFriction() const { return params.friction; }
    float GetAirResistance() const { return params.airResistance; }
    float GetLaunchVelocity() const { return params.launchVelocity; }
    float GetLaunchAngle() const { return params.launchAngle; }
};
