#pragma once
#include "GameMode.hpp"

class ClassicChessMode : public GameMode {
public:
    std::string getModeName() const override { return "Echecs classique"; }
    std::string getModeDescription() const override { 
        return "Règles classiques des échecs internationaux"; 
    }
};