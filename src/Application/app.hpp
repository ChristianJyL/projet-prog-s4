#pragma once
#include <imgui.h>
#include <iostream>
#include "Chess/Board.hpp"
#include "quick_imgui/quick_imgui.hpp"

class app {
public:
    void run();

private:
    int   m_width{};
    int   m_height{};
    Board m_board{};

    void drawBoard();
};