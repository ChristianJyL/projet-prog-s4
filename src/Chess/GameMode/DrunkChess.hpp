#pragma once
#include <imgui.h>
#include <map>
#include <random>
#include <vector>
#include "GameMode.hpp"

struct PlayerState {
    float alcoholLevel  = 0.0f; // Niveau d'alcool (0-100%)
    int   blackoutTurns = 0;  
};

struct BoardEffects {
    float boardTilt = 0.0f;
};

struct Bottle {
    Position position;
    float alcoholAmount; 
};

class DrunkChessMode : public GameMode {
public:
    DrunkChessMode();
    std::mt19937 m_random;

    // Méthodes d'identification du mode
    std::string getModeName() const override;
    std::string getModeDescription() const override;

    // Redéfinition uniquement des méthodes qui changent dans le mode bourré
    void initializeBoard(std::vector<Piece>& board) override;
    bool isValidMove(const std::vector<Piece>& board, Position from, Position to, const Piece& piece) override;
    void executeMove(std::vector<Piece>& board, Position from, Position to) override;
    void updatePerTurn(std::vector<Piece>& board, PieceColor currentTurn) override;

    ImVec4 getTileColor(bool isPairLine, int index, Position pos) const override;
    void   drawTileEffect(Position pos, ImVec2 cursorPos, Piece piece) const override;
    void   drawModeSpecificUI() override;

private:
    PlayerState  m_whitePlayerState; 
    PlayerState  m_blackPlayerState; 
    BoardEffects m_boardEffects;
    int          m_turnCount = 0;
    
    std::vector<Bottle> m_bottles;

    bool m_bottleCaptured = false;

    void   updateAlcoholLevels(PieceColor currentTurn);
    ImVec4 getAlcoholLevelColor(float level) const;
    float  getPlayerAlcoholLevel(PieceColor color) const;
    float  getAverageAlcoholLevel() const;
    bool   isPawnPromotion(Position to, Piece piece) const;
    void   trySpawnBottle(const std::vector<Piece>& board);
    bool   hasBottleAt(Position pos) const;
    void   removeBottleAt(Position pos);

    std::uniform_real_distribution<float> m_uniformDist{0.0f, 1.0f};
    std::normal_distribution<float>       m_normalDist{0.0f, 1.0f};
    std::bernoulli_distribution           m_bottleSpawnDist{0.5f}; 
};
