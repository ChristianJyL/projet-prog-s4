#pragma once

#include <vector>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "../utils/Geometry.hpp"
#include "../utils/Program.hpp"
#include "../Chess/Piece.hpp"
#include <map>

struct ChessPiece {
    PieceType type;
    PieceColor color;
    int x;
    int y;
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
    void render(const glm::mat4& view, const glm::mat4& projection, float squareSize);
    bool loadPieceModel(PieceType type, const std::string& modelPath);
    void addPiece(PieceType type, PieceColor color, int x, int y);
    void clearPieces();
    void cleanup();
    
    // Vérifier si des pièces sont disponibles pour le rendu
    bool hasPieces() const { return !m_pieces.empty(); }
    
    // Obtenir le nombre de pièces
    int getPieceCount() const { return m_pieces.size(); }
    
private:
    std::map<PieceType, PieceRenderData> m_pieceData;
    glBurnout::Program m_pieceShader;
    bool m_piecesLoaded;
    
    std::vector<ChessPiece> m_pieces;
    
    bool createShader();
    void setupBuffers(PieceType type, const glBurnout::Geometry& geometry);
};
