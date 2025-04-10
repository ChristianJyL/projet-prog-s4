#pragma once
#include <imgui.h>
#include <optional>
#include <vector>
#include <functional>
#include "Piece.hpp"
#include "Position.hpp"
#include "GameMode/GameMode.hpp" 
#include <memory> 


class Renderer3D;

class Board {
public:
    Board();

    void initializeBoard(Renderer3D* renderer = nullptr);
    Piece      get(Position pos) const;
    void       set(Position pos, Piece piece);
    void       move(Position from, Position to);
    void       executeMove(Position from, Position to);
    void       drawBoard();
    bool       isGameOver() const;
    PieceColor getWinner() const;
    
    //Pour le renderer3D
    const std::vector<Piece>& getBoardState() const { return m_list; }
    void setRenderer3D(Renderer3D* renderer) { m_renderer3D = renderer; }
    void syncCameraWithSelection();

    //Gamemode
    void setGameMode(std::unique_ptr<GameMode> mode);
    std::string getCurrentModeName() const;
    GameMode* getGameMode() const { return m_currentGameMode.get(); }

private:
    std::vector<Piece> m_list     = std::vector<Piece>(64);
    PieceColor         m_turn     = PieceColor::White; 
    PieceColor         m_winner   = PieceColor::White; // Couleur du joueur gagnant
    bool               m_gameOver = false;
    Renderer3D*        m_renderer3D = nullptr; 
    std::unique_ptr<GameMode> m_currentGameMode; 

    std::optional<Position> m_selectedPiece;      
    std::optional<Position> m_lastDoublePawnMove;

    bool       m_promotionInProgress = false;
    Position   m_promotionPosition;
    PieceColor m_promotionColor;
    
    void   drawTile(int index, bool pairLin, ImVec2& outCursorPos);
    ImVec4 getPieceColor(Piece piece) const;

    void handleMouseInteraction(int index);
    void handleClick(Position pos);
    void selectPiece(Position pos);
    void movePiece(Position pos);
    void nextTurn();

    bool                  isPathClear(Position from, Position to) const;
    bool                  isEnPassantCapture(Position from, Position to) const;
    std::vector<Position> getValidMoves(Position from) const;

    void drawPossibleMoves(Position pos, ImVec2 cursorPos, float tileSize);

    void handlePawnPromotion();
    bool isPawnPromotion(Position from, Position to, Piece piece) const;
};