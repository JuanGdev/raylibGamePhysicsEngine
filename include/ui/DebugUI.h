#pragma once
#include "raylib.h"
#include "core/GameObject.h"
#include <vector>
#include <string>

class DebugUI {
private:
    int debugWidth;
    int debugHeight;
    bool debugWindowOpen;
    Vector2 debugWindowPosition;
    
public:
    DebugUI(int width = 400, int height = 600);
    ~DebugUI();
    
    void Initialize();
    void Update();
    void Render(const GameObject& cube1, const GameObject& cube2, const std::vector<std::string>& messages);
    void Shutdown();
    
    bool IsOpen() const { return debugWindowOpen; }
    void SetOpen(bool open) { debugWindowOpen = open; }
    void ToggleWindow() { debugWindowOpen = !debugWindowOpen; }
};
