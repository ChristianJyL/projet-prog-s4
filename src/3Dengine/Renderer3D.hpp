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
    
    // Méthodes déléguées pour la caméra
    void setCameraAspectRatio(float ratio);
    glm::mat4 getProjectionMatrix() const { return m_camera.getProjectionMatrix(); }
    glm::mat4 getViewMatrix() const { return m_camera.getViewMatrix(); }
    CameraMode getCameraMode() const;

    
    void updatePiecesFromBoard(const Board& board);
    glm::vec3 getChessBoardPosition(int x, int y) const;
    
    // Sélection d'une pièce pour la vue en mode pièce
    bool selectPieceForView(int x, int y);
    void toggleCameraMode();
    
    // Méthodes pour la gestion des pièces suivies
    void updateTrackedPiece();
    
    // Méthodes déléguées pour les contrôles de caméra
    void moveCameraFront(float delta);
    void rotateCameraLeft(float degrees);
    void rotateCameraUp(float degrees);
    
private:
    Camera m_camera;
    
    SkyBox m_skybox;
    Chessboard m_chessboard;
    PieceRenderer m_pieceRenderer;
    bool m_isInitialized;
    
    glm::vec3 m_selectedPiecePosition;
    bool m_hasPieceSelected;
    PieceColor m_selectedPieceColor;
    
    int m_selectedPieceX;
    int m_selectedPieceY;
    
    bool initializeSkybox();
    bool initializeChessboard();
    bool initializePieces();
};