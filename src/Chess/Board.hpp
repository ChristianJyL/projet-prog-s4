#pragma once
#include <imgui.h>
#include <optional>
#include <vector>
#include "Piece.hpp"
#include "Position.hpp"

class Board {
public:
    void initializeBoard();
    Piece      get(Position pos) const;
    void       set(Position pos, Piece piece);
    void       move(Position from, Position to);
    void       drawBoard();
    bool       isGameOver() const;
    PieceColor getWinner() const { return m_winner; }

private:
    std::vector<Piece> m_list     = std::vector<Piece>(64);
    PieceColor         m_turn     = PieceColor::White; // Couleur du joueur actuel
    PieceColor         m_winner   = PieceColor::White; // Default winner, will be set when game ends
    bool               m_gameOver = false;

    

    std::optional<Position> m_selectedPiece;      // Stocke la pièce sélectionnée
    std::optional<Position> m_lastDoublePawnMove; // Stocke la position finale du dernier pion ayant avancé de deux cases

    // Variables pour la promotion des pions
    bool       m_promotionInProgress = false;
    Position   m_promotionPosition;
    PieceColor m_promotionColor;

    void   drawTile(int index, bool pairLin, ImVec2& outCursorPos);
    ImVec4 getPieceColor(Piece piece) const;
    ImVec4 getTileColor(bool isPairLine, int index) const; // color of tiles

    void handleMouseInteraction(int index);
    void handleClick(Position pos);
    void selectPiece(Position pos);
    void movePiece(Position pos);
    void nextTurn();

    bool                  isPathClear(Position from, Position to) const;
    bool                  isEnPassantCapture(Position from, Position to) const;
    std::vector<Position> getValidMoves(Position from) const;

    void drawPossibleMoves(Position pos, ImVec2 cursorPos);

    // Méthode pour gérer la promotion des pions
    void handlePawnPromotion();
    bool isPawnPromotion(Position from, Position to, Piece piece) const;
};