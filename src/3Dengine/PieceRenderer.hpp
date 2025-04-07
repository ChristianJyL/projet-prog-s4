#pragma once

#include <vector>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "../utils/Geometry.hpp"
#include "../utils/Program.hpp"
#include "../Chess/Piece.hpp"
#include <map>

// États possibles pour l'animation d'une pièce
enum class AnimationState {
    Idle,       // La pièce est immobile
    Moving,     // La pièce est en déplacement
    Capturing   // La pièce est en train de capturer une autre pièce
};

struct ChessPiece {
    PieceType type;
    PieceColor color;
    int x;
    int y;
    
    // Variables d'animation
    AnimationState state = AnimationState::Idle;
    float animationTime = 0.0f;          // Temps écoulé dans l'animation
    float animationDuration = 1.0f;      // Durée totale de l'animation
    
    // Position de départ et d'arrivée (pour les animations)
    glm::vec3 startPosition = glm::vec3(0.0f);
    glm::vec3 targetPosition = glm::vec3(0.0f);
    
    // Hauteur maximale pendant le déplacement
    float jumpHeight = 0.5f;
    
    // Indique si la pièce est en cours de capture
    bool isBeingCaptured = false;
};

struct PieceRenderData {
    glBurnout::Geometry geometry;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
};

class PieceRenderer {
public:
    PieceRenderer();
    ~PieceRenderer();
    
    bool initialize();
    void update(float deltaTime);
    void render(const glm::mat4& view, const glm::mat4& projection, float squareSize);
    bool loadPieceModel(PieceType type, const std::string& modelPath);
    void addPiece(PieceType type, PieceColor color, int x, int y);
    void clearPieces();
    void cleanup();
    
    // Méthodes pour l'animation des pièces
    void animateTransition(const std::vector<ChessPiece>& newState, float duration = 1.0f);
    bool isAnimating() const;
    
private:
    std::map<PieceType, PieceRenderData> m_pieceData;
    glBurnout::Program m_pieceShader;
    bool m_piecesLoaded;
    
    std::vector<ChessPiece> m_pieces;
    std::vector<ChessPiece> m_nextState; // État vers lequel on transite
    
    bool createShader();
    void setupBuffers(PieceType type, const glBurnout::Geometry& geometry);
    
    // Méthodes privées pour l'animation
    glm::vec3 calculateChessPosition(int x, int y, float squareSize) const;
    glm::mat4 calculateAnimatedModelMatrix(const ChessPiece& piece, float squareSize) const;
    void startPieceMovement(ChessPiece& piece, int targetX, int targetY, float duration);
};
