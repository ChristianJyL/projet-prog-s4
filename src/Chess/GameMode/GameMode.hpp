#pragma once
#include "../Position.hpp"
#include "../Piece.hpp"
#include <vector>
#include <string>
#include <imgui.h>

//je sais pas si c'est la meilleur façon de faire mais au moins je peux faire du polymorphisme sans trop répeter de code
class GameMode {
public:
    virtual ~GameMode() = default;
    
    virtual std::string getModeName() const { return "Mode de base"; }
    virtual std::string getModeDescription() const { return "Implémentation par défaut"; }
    
    virtual void initializeBoard(std::vector<Piece>& board);
    virtual bool isValidMove(const std::vector<Piece>& board, Position from, Position to, const Piece& piece);
    virtual void executeMove(std::vector<Piece>& board, Position from, Position to);
    virtual void updatePerTurn(std::vector<Piece>& board, PieceColor currentTurn) {}
    
    virtual void drawModeSpecificUI() {}
    virtual ImVec4 getTileColor(bool isPairLine, int index, Position pos) const;
    virtual void drawTileEffect(Position pos, ImVec2 cursorPos, Piece piece) const {}

private:
    bool isPathClear(const std::vector<Piece>& board, Position from, Position to) const;
};