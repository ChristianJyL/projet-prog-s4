#pragma once
#include <imgui.h>
#include <optional>
#include <vector>
#include "Position.hpp"
#include "Piece.hpp"


class Board {
public:
    Board();

    Piece get(Position pos) const;
    void set(Position pos, Piece piece);
    void move(Position from, Position to);
    void drawBoard();

private:
    std::vector<Piece> m_list  = std::vector<Piece>(64);
    PieceColor m_turn = PieceColor::White; // Couleur du joueur actuel

    void initializeBoard();

    std::optional<Position> m_selectedPiece;    // Stocke la pièce sélectionnée

    void drawTile(int index, bool pairLin , ImVec2& outCursorPos);
    ImVec4 getPieceColor(Piece piece) const;
    ImVec4 getTileColor(bool isPairLine, int index) const; // color of tiles


    void handleMouseInteraction(int index);
    void handleClick(Position pos);
    void selectPiece(Position pos);
    void movePiece(Position pos);
    void nextTurn();

    void drawPossibleMoves(Position pos, ImVec2 cursorPos);
};