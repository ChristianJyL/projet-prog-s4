#include "Board.hpp"
#include <imgui.h>
#include <vector>

Board::Board()
{
    // Initialize the board with the starting positions of the pieces
    // 'r' for rook, 'n' for knight, 'b' for bishop, 'q' for queen, 'k' for king, 'p' for pawn
    // Uppercase for white pieces, lowercase for black pieces
    Board::m_list = {
        'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r',
        'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p',
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P',
        'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'
    };
}

ImVec4 Board::getColor(char piece)
{
    if (piece >= 'a' && piece <= 'z')
    {
        return {0.0f, 0.0f, 0.0f, 1.0f}; // black
    }
    else
    {
        return {1.0f, 1.0f, 1.0f, 1.0f}; // white
    }
}