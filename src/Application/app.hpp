#pragma once

#include "Chess/Board.hpp"
#include "3DRenderer/Renderer3D.hpp"

class app {
public:
    void init();
    void update();
    
private:
    Board m_board;
    Renderer3D m_renderer3D;
};