#pragma once
#include <imgui.h>
#include "Chess/Board.hpp"

class app {
public:
    void run();

private:
    int   m_width{8};
    int   m_height{8};
    Board m_board{};

    void drawBoard(const std::vector<char>& board);
};