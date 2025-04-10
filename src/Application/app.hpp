#pragma once

#include "Chess/Board.hpp"
#include "3Dengine/Renderer3D.hpp"

class app {
public:
    void init();
    void update();

private:
    Board m_board;
    Renderer3D m_renderer3D;
    
    void drawGameModeWindow();
    void drawCameraControlWindow();
    void draw3DViewportWindow();
    void drawGameOverPopup(bool& gameOverPopupClosed);
};