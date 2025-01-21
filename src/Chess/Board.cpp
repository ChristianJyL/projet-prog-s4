#include "Board.hpp"
#include <vector>

void Board::init()
{
    Board::m_board = std::vector<int>(64, 0);
}