#pragma once

#include <glm/glm.hpp>
#include "SkyBox.hpp"
#include "Cameras/Camera.hpp"
#include "Chessboard.hpp"
#include "PieceRenderer.hpp"
#include "../Chess/Board.hpp"

class Renderer3D {
public:
    Renderer3D();
    ~Renderer3D();
    
    bool initialize();
    bool isInitialized() const;
    void update(float deltaTime);
    void render();
    void cleanup();
    
    // Accès direct à la caméra
    Camera& getCamera() { return m_camera; }
    const Camera& getCamera() const { return m_camera; }
    
    // Méthode pour mettre à jour les pièces 3D selon l'état de l'échiquier 2D
    void updatePiecesFromBoard(const Board& board);
    
    // Obtenir la position 3D d'une case de l'échiquier
    glm::vec3 getChessBoardPosition(int x, int y) const;
    
    // Sélection d'une pièce pour la vue en mode pièce
    bool selectPieceForView(int x, int y);
    void toggleCameraMode();
    
    // Méthodes pour la gestion des pièces suivies
    void updateTrackedPiece();
    
private:
    // Caméra standard
    Camera m_camera;
    
    SkyBox* m_skybox;
    Chessboard* m_chessboard;
    PieceRenderer* m_pieceRenderer;
    bool m_isInitialized;
    
    // Position de la pièce sélectionnée pour la vue
    glm::vec3 m_selectedPiecePosition;
    bool m_hasPieceSelected;
    
    // Couleur de la pièce sélectionnée pour la vue
    PieceColor m_selectedPieceColor;
    
    // Coordonnées de la pièce sélectionnée dans l'échiquier
    int m_selectedPieceX;
    int m_selectedPieceY;
    
    // Initialisation des différentes parties
    bool initializeSkybox();
    bool initializeChessboard();
    bool initializePieces();
};