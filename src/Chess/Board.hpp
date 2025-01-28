#pragma once
#include <imgui.h>
#include <vector>

class Board {
public:
    std::vector<char> m_list;
    Board();
    ImVec4 getColor(char piece);
};