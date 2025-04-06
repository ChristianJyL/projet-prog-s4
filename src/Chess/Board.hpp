#pragma once
#include <imgui.h>
#include <optional>
#include <vector>
#include <functional>
#include "Piece.hpp"
#include "Position.hpp"

// Prédéclaration de la classe Renderer3D pour éviter l'inclusion circulaire
class Renderer3D;

class Board {
public:
    // Définir un type pour les callbacks de changement d'état
    using BoardChangeCallback = std::function<void(const Board&)>;

    void initializeBoard();
    Piece      get(Position pos) const;
    void       set(Position pos, Piece piece);
    void       move(Position from, Position to);
    void       drawBoard();
    bool       isGameOver() const;
    PieceColor getWinner() const { return m_winner; }
    
    // Méthode pour obtenir l'état complet de l'échiquier
    const std::vector<Piece>& getBoardState() const { return m_list; }
    
    // Méthode pour ajouter un callback de changement d'état
    void addBoardChangeCallback(BoardChangeCallback callback) {
        m_changeCallbacks.push_back(callback);
    }

    // Définir le renderer 3D à utiliser
    void setRenderer3D(Renderer3D* renderer) { m_renderer3D = renderer; }
    
    // Mise à jour du rendu 3D
    void updateRenderer3D();

private:
    std::vector<Piece> m_list     = std::vector<Piece>(64);
    PieceColor         m_turn     = PieceColor::White; // Couleur du joueur actuel
    PieceColor         m_winner   = PieceColor::White; // Default winner, will be set when game ends
    bool               m_gameOver = false;
    Renderer3D*        m_renderer3D = nullptr; // Pointeur vers le renderer 3D

    std::optional<Position> m_selectedPiece;      // Stocke la pièce sélectionnée
    std::optional<Position> m_lastDoublePawnMove; // Stocke la position finale du dernier pion ayant avancé de deux cases

    // Variables pour la promotion des pions
    bool       m_promotionInProgress = false;
    Position   m_promotionPosition;
    PieceColor m_promotionColor;
    
    // Liste des callbacks à appeler lors des changements d'état
    std::vector<BoardChangeCallback> m_changeCallbacks;

    void   drawTile(int index, bool pairLin, ImVec2& outCursorPos);
    ImVec4 getPieceColor(Piece piece) const;
    ImVec4 getTileColor(bool isPairLine, int index) const; // color of tiles

    void handleMouseInteraction(int index);
    void handleClick(Position pos);
    void selectPiece(Position pos);
    void movePiece(Position pos);
    void nextTurn();
    
    // Méthode pour notifier les observateurs des changements
    void notifyBoardChanged();

    bool                  isPathClear(Position from, Position to) const;
    bool                  isEnPassantCapture(Position from, Position to) const;
    std::vector<Position> getValidMoves(Position from) const;

    void drawPossibleMoves(Position pos, ImVec2 cursorPos);

    // Méthode pour gérer la promotion des pions
    void handlePawnPromotion();
    bool isPawnPromotion(Position from, Position to, Piece piece) const;
};