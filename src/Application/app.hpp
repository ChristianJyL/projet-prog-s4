#pragma once
#include <imgui.h>
#include "Chess/Board.hpp"

class app {
private:
    Board m_board;

public:
    void run();
};