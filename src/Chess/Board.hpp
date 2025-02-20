#pragma once
#include <imgui.h>
#include <optional>
#include <vector>

struct Position {
    int x;
    int y;
};

class Board {
public:
    void drawBoard();

private:
    std::vector<char> m_list = {
        'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r',
        'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p',
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P',
        'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'
    };
    bool                    m_whiteTurn = true; // true = blanc, false = noir
    std::optional<Position> m_selectedPiece;    // Stocke la pièce sélectionnée

    ImVec4 getTileColor(bool isPairLine, int index) const; // color of tiles
    ImVec4 getColor(char piece);                           // Color of pieces

    void drawTile(int index, bool pairLine);
    void handleMouseInteraction(int index);

    char get(int x, int y);
    void set(int x, int y, char piece);
    void move(Position from, Position to);

    void handleClick(int x, int y);
    void selectPiece(int x, int y);
    void movePiece(int x, int y);
    void nextTurn();

    void showPossibleMoves();
};