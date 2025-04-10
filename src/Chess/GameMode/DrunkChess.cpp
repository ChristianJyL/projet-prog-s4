#include "DrunkChess.hpp"
#include <algorithm>
#include <array>
#include <random>
#include <ctime>
#include <iostream>

DrunkChessMode::DrunkChessMode()
    : m_random(std::random_device()())
{
    // Initialiser les joueurs sobres
    m_whitePlayerState = {0.0f, 0};
    m_blackPlayerState = {0.0f, 0};
    m_bottleCaptured = false;
}

std::string DrunkChessMode::getModeName() const {
    return "Échecs Bourrés";
}

std::string DrunkChessMode::getModeDescription() const {
    return "Mode où les joueurs consomment de l'alcool virtuel, affectant leur précision et visibilité";
}

void DrunkChessMode::initializeBoard(std::vector<Piece>& board)
{
    GameMode::initializeBoard(board);

    m_boardEffects = BoardEffects();
    
    m_whitePlayerState = {0.0f, 0};
    m_blackPlayerState = {0.0f, 0};
    
    m_bottles.clear();
}

bool DrunkChessMode::isValidMove(const std::vector<Piece>& board, Position from, Position to, const Piece& piece) {
    PlayerState& currentPlayerState = (piece.color == PieceColor::White) ? 
                                    m_whitePlayerState : m_blackPlayerState;
    
    if (currentPlayerState.blackoutTurns > 0) {
        return false; // Le joueur est trop bourré pour bouger
    }
    
    //sinon on utilise la méthode classique
    return GameMode::isValidMove(board, from, to, piece);
}

void DrunkChessMode::executeMove(std::vector<Piece>& board, Position from, Position to) {
    Piece piece = board[from.x + from.y * 8];
    PlayerState& currentPlayerState = (piece.color == PieceColor::White) ? 
                                    m_whitePlayerState : m_blackPlayerState;
    
    // Case d'arrivée prévue (peut être modifiée si le joueur est bourré)
    Position actualTo = to;
    
    // Si le joueur est bourré, chance d'imprécision dans le mouvement
    if (currentPlayerState.alcoholLevel > 20.0f) {
        // Calculer la probabilité et l'amplitude de l'imprécision
        float alcoholEffect = currentPlayerState.alcoholLevel / 100.0f; // 0.2 - 1.0
        
        // Probabilité d'imprécision augmente avec le niveau d'alcool
        if (m_uniformDist(m_random) < alcoholEffect * 0.8f) {
            int maxDeviation = 1;
            if (currentPlayerState.alcoholLevel > 70.0f) maxDeviation = 2;
            
            // Générer des déviations aléatoires
            int deviationX = std::uniform_int_distribution<int>(-maxDeviation, maxDeviation)(m_random);
            int deviationY = std::uniform_int_distribution<int>(-maxDeviation, maxDeviation)(m_random);
            
            // Appliquer la déviation, mais vérifier que la position reste valide
            Position deviatedPos = {to.x + deviationX, to.y + deviationY};
            
            if (deviatedPos.isValid()) {
                // Vérifier si la case déviée ne contient pas une pièce alliée
                Piece targetPiece = board[deviatedPos.x + deviatedPos.y * 8];
                if (targetPiece.type == PieceType::None || targetPiece.color != piece.color) {
                    actualTo = deviatedPos;
                }
            }
        }
    }
    
    Piece capturedPiece = board[actualTo.x + actualTo.y * 8];
    bool needsPromotion = isPawnPromotion(actualTo, piece);
    
    bool capturedBottle = hasBottleAt(actualTo);
    float bottleAlcoholAmount = 0.0f;
    
    if (capturedBottle) {
        for (const auto& bottle : m_bottles) {
            if (bottle.position.x == actualTo.x && bottle.position.y == actualTo.y) {
                bottleAlcoholAmount = bottle.alcoholAmount;
                break;
            }
        }
        removeBottleAt(actualTo);
    }
    
    GameMode::executeMove(board, from, actualTo);
    
    // Augmenter l'alcoolémie après un mouvement
    float alcoholIncrease = std::uniform_real_distribution<float>(1.0f, 5.0f)(m_random);
    
    if (capturedPiece.type != PieceType::None) {
        alcoholIncrease += 3.0f;
    }
    
    // Si le joueur a promu un pion, augmenter l'alcoolémie
    if (needsPromotion) {
        alcoholIncrease += 5.0f;
    }
    
    if (capturedBottle) {
        alcoholIncrease += bottleAlcoholAmount;
        m_bottleCaptured = true;
    }
    
    currentPlayerState.alcoholLevel += alcoholIncrease;
    if (currentPlayerState.alcoholLevel > 100.0f) {
        currentPlayerState.alcoholLevel = 100.0f;
        
        // Chance d'entrer en blackout si très bourré
        if (currentPlayerState.alcoholLevel > 80.0f && m_uniformDist(m_random) < 0.3f) {
            currentPlayerState.blackoutTurns = std::uniform_int_distribution<int>(1, 2)(m_random);
        }
    }
}

bool DrunkChessMode::isPawnPromotion(Position to, Piece piece) const {
    if (piece.type != PieceType::Pawn)
        return false;

    // Un pion blanc qui atteint la rangée 7
    if (piece.color == PieceColor::White && to.y == 7)
        return true;

    // Un pion noir qui atteint la rangée 0
    if (piece.color == PieceColor::Black && to.y == 0)
        return true;

    return false;
}

void DrunkChessMode::updatePerTurn(std::vector<Piece>& board, PieceColor currentTurn) {
    m_turnCount++;
    
    // Mise à jour des niveaux d'alcool et des états de blackout
    updateAlcoholLevels(currentTurn);
    
    // Inclinaison du plateau proportionnelle à l'alcoolémie moyenne
    float avgAlcohol = getAverageAlcoholLevel();
    m_boardEffects.boardTilt = m_normalDist(m_random) * 0.1f * (avgAlcohol / 100.0f);
    
    // Essayer de faire apparaître une bouteille (utilisant la loi de Bernoulli)
    trySpawnBottle(board);
}

void DrunkChessMode::trySpawnBottle(const std::vector<Piece>& board) {
    //std::cout << "Tentative de création d'une bouteille..." << std::endl;
    //la loi de Bernoulli
    if (m_bottleSpawnDist(m_random)) {
        // Trouver les cases vides
        std::vector<Position> emptyPositions;
        
        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 8; ++x) {
                Position pos = {x, y};
                if (board[pos.x + pos.y * 8].type == PieceType::None && !hasBottleAt(pos)) {
                    emptyPositions.push_back(pos);
                }
            }
        }
        // S'il y a des cases vides, on place une bouteille
        if (!emptyPositions.empty()) {
            // Choisir une position aléatoire parmi les cases vides
            int index = std::uniform_int_distribution<int>(0, static_cast<int>(emptyPositions.size()) - 1)(m_random);
            Position bottlePos = emptyPositions[index];
            
            // (entre 5 et 15%)
            float alcoholAmount = std::uniform_real_distribution<float>(5.0f, 15.0f)(m_random);
            
            // Ajouter la bouteille
            //std::cout << "Bouteille créée à la position: " << bottlePos.x << ", " << bottlePos.y << std::endl;
            m_bottles.push_back({bottlePos, alcoholAmount});
        }
    }
}

bool DrunkChessMode::hasBottleAt(Position pos) const {
    for (const auto& bottle : m_bottles) {
        if (bottle.position.x == pos.x && bottle.position.y == pos.y) {
            return true;
        }
    }
    return false;
}

void DrunkChessMode::removeBottleAt(Position pos) {
    m_bottles.erase(
        std::remove_if(
            m_bottles.begin(),
            m_bottles.end(),
            [pos](const Bottle& bottle) { 
                return bottle.position.x == pos.x && bottle.position.y == pos.y; 
            }
        ),
        m_bottles.end()
    );
}

float DrunkChessMode::getPlayerAlcoholLevel(PieceColor color) const {
    return (color == PieceColor::White) ? m_whitePlayerState.alcoholLevel : m_blackPlayerState.alcoholLevel;
}

void DrunkChessMode::updateAlcoholLevels(PieceColor currentTurn) {
    // Légère diminution de l'alcoolémie pour les deux joueurs (ils se déssoûlent lentement)
    m_whitePlayerState.alcoholLevel = std::max(0.0f, m_whitePlayerState.alcoholLevel - 0.2f);
    m_blackPlayerState.alcoholLevel = std::max(0.0f, m_blackPlayerState.alcoholLevel - 0.2f);
    
    // Diminuer les tours de blackout si nécessaire
    if (m_whitePlayerState.blackoutTurns > 0) {
        m_whitePlayerState.blackoutTurns--;
    }
    if (m_blackPlayerState.blackoutTurns > 0) {
        m_blackPlayerState.blackoutTurns--;
    }
    
    // Augmenter légèrement l'alcool du joueur actuel (boit entre les coups)
    PlayerState& currentPlayerState = (currentTurn == PieceColor::White) ? 
                                    m_whitePlayerState : m_blackPlayerState;
    
    // Boire pendant qu'on attend son tour
    float drinkAmount = std::uniform_real_distribution<float>(0.1f, 0.5f)(m_random);
    currentPlayerState.alcoholLevel += drinkAmount;
    
    if (currentPlayerState.alcoholLevel > 100.0f) {
        currentPlayerState.alcoholLevel = 100.0f;
    }
}

float DrunkChessMode::getAverageAlcoholLevel() const {
    return (m_whitePlayerState.alcoholLevel + m_blackPlayerState.alcoholLevel) / 2.0f;
}

ImVec4 DrunkChessMode::getTileColor(bool isPairLine, int index, Position pos) const {
    float avgAlcohol = getAverageAlcoholLevel();
    
    bool isLightSquare = (isPairLine && index % 2 == 0) || (!isPairLine && index % 2 != 0);
    ImVec4 baseColor = isLightSquare ? 
        ImVec4(0.9f, 0.9f, 0.7f, 1.0f) :   // Beige clair
        ImVec4(0.5f, 0.3f, 0.1f, 1.0f);    // Marron foncé
    
    // Sans effet à faible niveau d'alcool
    if (avgAlcohol < 3.0f) {
        return baseColor;
    }
    
    // Utiliser directement le temps de ImGui pour un changement visible
    float time = ImGui::GetTime() * 1.2f;  
    
    // Calculer l'intensité de l'effet basée sur l'alcoolémie moyenne
    float alcoholEffect = std::min(avgAlcohol / 100.0f * 1.5f, 1.0f);  // Effet plus intense
    
    float xOffset = pos.x * 0.4f;
    float yOffset = pos.y * 0.6f;
    
    float r, g, b;
    
    r = 0.5f + 0.5f * std::sin(time + xOffset);
    g = 0.5f + 0.5f * std::sin(time * 1.3f + yOffset + 2.0f);
    b = 0.5f + 0.5f * std::sin(time * 0.7f + xOffset + yOffset + 4.0f);
    
    // Effet de vague qui traverse l'échiquier
    float wave = std::sin(time * 0.4f + (pos.x + pos.y) * 0.3f) * 0.15f;
    
    // Mélange entre la couleur de base et la couleur psychédélique 
    ImVec4 resultColor;
    resultColor.x = std::clamp(baseColor.x * (1.0f - alcoholEffect) + r * alcoholEffect + wave, 0.0f, 1.0f);
    resultColor.y = std::clamp(baseColor.y * (1.0f - alcoholEffect) + g * alcoholEffect - wave * 0.5f, 0.0f, 1.0f);
    resultColor.z = std::clamp(baseColor.z * (1.0f - alcoholEffect) + b * alcoholEffect + wave * 0.7f, 0.0f, 1.0f);
    resultColor.w = 1.0f;
    
    // Pour conserver la structure de l'échiquier même à des niveaux élevés d'alcool
    if (!isLightSquare) {
        resultColor.x *= 0.7f;
        resultColor.y *= 0.7f;
        resultColor.z *= 0.7f;
    }
    
    return resultColor;
}

void DrunkChessMode::drawTileEffect(Position pos, ImVec2 cursorPos, Piece piece) const {
    // Dessiner les effets des pièces qui oscillent
    if (piece.type != PieceType::None) {
        float alcoholLevel = getPlayerAlcoholLevel(piece.color);
        
        // que si le joueur est au moins un peu bourré
        if (alcoholLevel > 10.0f) {
            float oscillation = std::min((alcoholLevel - 10.0f) / 90.0f, 1.0f);
            float time = ImGui::GetTime() * (2.0f + oscillation * 2.0f); 
            
            float offsetX = std::sin(time) * oscillation * 12.0f;
            float offsetY = std::cos(time * 1.3f) * oscillation * 10.0f;
            
            ImGui::GetWindowDrawList()->AddText(
                ImVec2(cursorPos.x + 25 + offsetX, cursorPos.y + 25 + offsetY),
                piece.color == PieceColor::White ? IM_COL32(255, 255, 255, 255) : IM_COL32(0, 0, 0, 255),
                std::string(1, piece.toChar()).c_str()
            );
        }
    }
    
    // Dessiner les bouteilles sur les cases vides
    for (const auto& bottle : m_bottles) {
        if (bottle.position.x == pos.x && bottle.position.y == pos.y) {
            // Dessiner un cercle pour représenter une bouteille
            ImGui::GetWindowDrawList()->AddCircleFilled(
                ImVec2(cursorPos.x + 30, cursorPos.y + 30), // pas vraiment le cercle mais il est bouré
                15.0f,                                      
                IM_COL32(0, 0, 255, 150)                   //  bleu 
            );
            ImGui::GetWindowDrawList()->AddText(
                ImVec2(cursorPos.x + 25, cursorPos.y + 25),
                IM_COL32(255, 255, 255, 255),
                "B"
            );
        }
    }
}

// Colorisation basée sur le niveau d'alcool
ImVec4 DrunkChessMode::getAlcoholLevelColor(float level) const {
    if (level <= 20.0f) {
        return ImVec4{0.0f, 1.0f, 0.0f, 1.0f}; // Vert = sobre
    }
    else if (level <= 50.0f) {
        // Jaune-orange = pompette
        return ImVec4{1.0f, 0.8f, 0.0f, 1.0f};
    }
    else if (level <= 80.0f) {
        // Orange-rouge = saoul
        return ImVec4{1.0f, 0.4f, 0.0f, 1.0f};
    }
    else {
        // Rouge foncé = blackout
        return ImVec4{0.8f, 0.0f, 0.0f, 1.0f};
    }
}

void DrunkChessMode::drawModeSpecificUI() {
    if (!ImGui::Begin("Mode Bourrés - État des joueurs")) {
        ImGui::End();
        return;
    }
    
    // Statistiques du mode
    ImGui::Text("Tour: %d", m_turnCount);
    ImGui::Separator();
    
    // État du joueur blanc
    ImGui::TextColored(getAlcoholLevelColor(m_whitePlayerState.alcoholLevel), 
                       "Joueur Blanc: %.1f%% d'alcoolémie", 
                       m_whitePlayerState.alcoholLevel);
    
    ImGui::SameLine();
    
    if (m_whitePlayerState.blackoutTurns > 0) {
        ImGui::TextColored(ImVec4(1,0,0,1), "(BLACKOUT! %d tours)", m_whitePlayerState.blackoutTurns);
    } else if (m_whitePlayerState.alcoholLevel > 80.0f) {
        ImGui::TextColored(ImVec4(1,0,0,1), "(Complètement bourré!)");
    } else if (m_whitePlayerState.alcoholLevel > 50.0f) {
        ImGui::TextColored(ImVec4(1,0.5f,0,1), "(Saoul)");
    } else if (m_whitePlayerState.alcoholLevel > 20.0f) {
        ImGui::TextColored(ImVec4(1,1,0,1), "(Pompette)");
    } else {
        ImGui::TextColored(ImVec4(0,1,0,1), "(Sobre)");
    }
    
    ImGui::ProgressBar(m_whitePlayerState.alcoholLevel / 100.0f, ImVec2(-1, 10), "");
    
    ImGui::Spacing();
    
    // État du joueur noir
    ImGui::TextColored(getAlcoholLevelColor(m_blackPlayerState.alcoholLevel), 
                       "Joueur Noir: %.1f%% d'alcoolémie", 
                       m_blackPlayerState.alcoholLevel);
    
    ImGui::SameLine();
    
    if (m_blackPlayerState.blackoutTurns > 0) {
        ImGui::TextColored(ImVec4(1,0,0,1), "(BLACKOUT! %d tours)", m_blackPlayerState.blackoutTurns);
    } else if (m_blackPlayerState.alcoholLevel > 80.0f) {
        ImGui::TextColored(ImVec4(1,0,0,1), "(Complètement bourré!)");
    } else if (m_blackPlayerState.alcoholLevel > 50.0f) {
        ImGui::TextColored(ImVec4(1,0.5f,0,1), "(Saoul)");
    } else if (m_blackPlayerState.alcoholLevel > 20.0f) {
        ImGui::TextColored(ImVec4(1,1,0,1), "(Pompette)");
    } else {
        ImGui::TextColored(ImVec4(0,1,0,1), "(Sobre)");
    }
    
    ImGui::ProgressBar(m_blackPlayerState.alcoholLevel / 100.0f, ImVec2(-1, 10), "");
    
    ImGui::Spacing();
    ImGui::Separator();
    
    // Afficher le nombre de bouteilles sur le plateau
    ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), "Bouteilles sur le plateau: %d", static_cast<int>(m_bottles.size()));
    
    ImGui::TextColored(ImVec4(1,1,0,1), "Effets de l'alcool:");
    ImGui::BulletText("0-20%%: Sobre - Déplacements normaux");
    ImGui::BulletText("21-50%%: Pompette - Légère oscillation des pièces");
    ImGui::BulletText("51-80%%: Saoul - Déplacements imprécis possibles");
    ImGui::BulletText(">80%%: Risque de blackout (impossible de jouer pour 1-2 tours)");
    ImGui::BulletText("Bouteilles: Des bouteilles d'alcool peuvent apparaître au hasard!");
    
    if (m_bottleCaptured) {
        ImGui::OpenPopup("Bouteille capturée!");
        m_bottleCaptured = false;
    }
    
    if (ImGui::BeginPopupModal("Bouteille capturée !!!", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Vous avez capturé une bouteille et bu son contenu !");
        ImGui::Text("Votre taux d'alcoolémie a augmenté!");
        
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    
    ImGui::End();
}