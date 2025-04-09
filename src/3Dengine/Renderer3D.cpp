#include "Renderer3D.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include "../Chess/Board.hpp"

Renderer3D::Renderer3D() 
    : m_isInitialized(false),
      m_selectedPiecePosition(0.0f), m_hasPieceSelected(false), m_selectedPieceColor(PieceColor::White),
      m_selectedPieceX(0), m_selectedPieceY(0),
      m_chessboard(8) // Initialiser l'échiquier avec une taille de 8x8
{
    std::cout << "Renderer3D constructor called" << std::endl;
}

Renderer3D::~Renderer3D() {
    cleanup();
}

bool Renderer3D::initialize() {
    if (m_isInitialized) {
        return true; // Éviter une réinitialisation si déjà initialisé
    }

    std::cout << "Initializing 3D Renderer..." << std::endl;

    try {
        // Initialiser les différents composants
        if (!initializeSkybox() || !initializeChessboard() || !initializePieces()) {
            cleanup();
            return false;
        }
        // Initialiser la caméra
        m_camera.setInitialPosition();

        m_isInitialized = true;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "ERROR::RENDERER3D::INITIALIZATION_EXCEPTION: " << e.what() << std::endl;
        cleanup();
        return false;
    }
}

bool Renderer3D::initializeSkybox() {
    // Initialiser la skybox
    if (!m_skybox.initialize()) {
        std::cerr << "ERROR::RENDERER3D::SKYBOX_INITIALIZATION_FAILED" << std::endl;
        return false;
    }
    std::cout << "Skybox created successfully." << std::endl;
    return true;
}

bool Renderer3D::initializeChessboard() {
    // Initialiser l'échiquier 3D (déjà créé dans le constructeur)
    if (!m_chessboard.initialize()) {
        std::cerr << "ERROR::RENDERER3D::CHESSBOARD_INITIALIZATION_FAILED" << std::endl;
        return false;
    }
    std::cout << "Chessboard created successfully." << std::endl;
    return true;
}

bool Renderer3D::initializePieces() {
    // Initialiser le gestionnaire de pièces
    if (!m_pieceRenderer.initialize()) {
        std::cerr << "ERROR::RENDERER3D::PIECE_RENDERER_INITIALIZATION_FAILED" << std::endl;
        return false;
    }
    
    // Structure pour simplifier le chargement des modèles
    struct PieceModelInfo {
        PieceType type;
        std::string path;
    };
    
    // Définir les chemins pour chaque type de pièce
    std::vector<PieceModelInfo> pieceModels = {
        {PieceType::Pawn, "../../assets/models/Piece/pawn.obj"},
        {PieceType::Rook, "../../assets/models/Piece/rook.obj"},
        {PieceType::Knight, "../../assets/models/Piece/knight.obj"},
        {PieceType::Bishop, "../../assets/models/Piece/bishop.obj"},
        {PieceType::Queen, "../../assets/models/Piece/queen.obj"},
        {PieceType::King, "../../assets/models/Piece/king.obj"}
    };
    
    // Vérifier que le dossier des modèles existe
    std::string modelDirPath = "../../assets/models/Piece";
    std::ifstream dirCheck(modelDirPath + "/.directory_check");
    if (!dirCheck.good()) {
        std::cerr << "WARNING: The models directory may not exist: " << modelDirPath << std::endl;
        std::cerr << "Please ensure the models are in the correct location relative to the executable." << std::endl;
        std::cerr << "Current working directory structure should be checked." << std::endl;
    }
    
    // Charger tous les modèles
    bool anyLoaded = false;
    for (const auto& pieceInfo : pieceModels) {
        // Vérifier si le fichier existe avant d'essayer de le charger
        std::ifstream fileCheck(pieceInfo.path);
        if (!fileCheck.good()) {
            std::cerr << "WARNING: Model file does not exist: " << pieceInfo.path << std::endl;
            continue;
        }
        
        if (m_pieceRenderer.loadPieceModel(pieceInfo.type, pieceInfo.path)) {
            anyLoaded = true;
            std::cout << "Successfully loaded model for " << static_cast<int>(pieceInfo.type) << std::endl;
        } else {
            std::cerr << "WARNING: Failed to load model for piece type " << static_cast<int>(pieceInfo.type) << std::endl;
        }
    }
    
    if (!anyLoaded) {
        std::cerr << "WARNING: Failed to load any piece model. Chess pieces will not be displayed." << std::endl;
        // Continuer même si le chargement échoue
    } else {
        // Effacer les pièces existantes
        m_pieceRenderer.clearPieces();
        
        // Placer les pièces blanches
        // Pions (rang 1)
        for (int i = 0; i < 8; i++) {
            m_pieceRenderer.addPiece(PieceType::Pawn, PieceColor::White, i, 1);
        }
        
        // Tours (coins)
        m_pieceRenderer.addPiece(PieceType::Rook, PieceColor::White, 0, 0);
        m_pieceRenderer.addPiece(PieceType::Rook, PieceColor::White, 7, 0);
        
        // Cavaliers (à côté des tours)
        m_pieceRenderer.addPiece(PieceType::Knight, PieceColor::White, 1, 0);
        m_pieceRenderer.addPiece(PieceType::Knight, PieceColor::White, 6, 0);
        
        // Fous (à côté des cavaliers)
        m_pieceRenderer.addPiece(PieceType::Bishop, PieceColor::White, 2, 0);
        m_pieceRenderer.addPiece(PieceType::Bishop, PieceColor::White, 5, 0);
        
        // Reine (à gauche du roi)
        m_pieceRenderer.addPiece(PieceType::Queen, PieceColor::White, 3, 0);
        
        // Roi (au centre)
        m_pieceRenderer.addPiece(PieceType::King, PieceColor::White, 4, 0);
        
        // Placer les pièces noires
        // Pions (rang 6)
        for (int i = 0; i < 8; i++) {
            m_pieceRenderer.addPiece(PieceType::Pawn, PieceColor::Black, i, 6);
        }
        
        // Tours (coins)
        m_pieceRenderer.addPiece(PieceType::Rook, PieceColor::Black, 0, 7);
        m_pieceRenderer.addPiece(PieceType::Rook, PieceColor::Black, 7, 7);
        
        // Cavaliers (à côté des tours)
        m_pieceRenderer.addPiece(PieceType::Knight, PieceColor::Black, 1, 7);
        m_pieceRenderer.addPiece(PieceType::Knight, PieceColor::Black, 6, 7);
        
        // Fous (à côté des cavaliers)
        m_pieceRenderer.addPiece(PieceType::Bishop, PieceColor::Black, 2, 7);
        m_pieceRenderer.addPiece(PieceType::Bishop, PieceColor::Black, 5, 7);
        
        // Reine (à gauche du roi)
        m_pieceRenderer.addPiece(PieceType::Queen, PieceColor::Black, 3, 7);
        
        // Roi (au centre)
        m_pieceRenderer.addPiece(PieceType::King, PieceColor::Black, 4, 7);
    }
    
    std::cout << "Piece renderer created successfully." << std::endl;
    return true;
}

bool Renderer3D::isInitialized() const {
    return m_isInitialized;
}

void Renderer3D::update(float deltaTime) {
    m_camera.update(deltaTime);
    
    // Mettre à jour les animations des pièces
    m_pieceRenderer.update(deltaTime);
    
    // Si une pièce est sélectionnée en mode vue pièce, mettre à jour sa position
    if (m_hasPieceSelected && m_camera.getCameraMode() == CameraMode::Piece) {
        updateTrackedPiece();
    }
}

void Renderer3D::cleanup() {
    // Pas besoin de supprimer des objets, ils seront détruits automatiquement
    // quand l'instance de Renderer3D sera détruite
    
    m_isInitialized = false;
}

void Renderer3D::render() {
    if (!m_isInitialized) {
        std::cerr << "WARNING: Renderer3D n'est pas initialisé, render() ignoré" << std::endl;
        return;
    }
    
    // Obtenir les matrices de vue et de projection de la caméra
    glm::mat4 viewMatrix = m_camera.getViewMatrix();
    glm::mat4 projectionMatrix = m_camera.getProjectionMatrix();
    
    // Activer le test de profondeur pour le rendu 3D
    glEnable(GL_DEPTH_TEST);
    
    // Nettoyer les buffers avant de dessiner avec une couleur de fond distinctive
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);  // Couleur de fond bleu-vert foncé
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Vérifier s'il y a des erreurs avant le rendu
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error before rendering: 0x" << std::hex << err << std::dec << std::endl;
    }
    
    // Rendre l'échiquier 3D
    m_chessboard.render(viewMatrix, projectionMatrix);
    
    // Rendre les pièces d'échecs
    m_pieceRenderer.render(viewMatrix, projectionMatrix, m_chessboard.getSquareSize());
    
    // Rendre la skybox en dernier (enveloppe tout)
    m_skybox.Draw(viewMatrix, projectionMatrix);
    
    // Vérifier les erreurs OpenGL après le rendu complet
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error after rendering: 0x" << std::hex << err << std::dec << std::endl;
    }
}

void Renderer3D::updatePiecesFromBoard(const Board& board) {
    if (!m_isInitialized) {
        return;
    }
    
    // Récupérer l'état actuel de l'échiquier
    const std::vector<Piece>& boardState = board.getBoardState();
    std::vector<ChessPiece> newState;
    
    // Convertir l'état 2D en représentation 3D
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            int index = x + y * 8;
            Piece piece = boardState[index];
            
            if (piece.type != PieceType::None) {
                ChessPiece chessPiece;
                chessPiece.type = piece.type;
                chessPiece.color = piece.color;
                chessPiece.x = x;
                chessPiece.y = y;
                
                newState.push_back(chessPiece);
            }
        }
    }
    
    // Animer la transition vers le nouvel état
    m_pieceRenderer.animateTransition(newState, 1.0f);
}

glm::vec3 Renderer3D::getChessBoardPosition(int x, int y) const {
    float squareSize = m_chessboard.getSquareSize();
    float squareHeight = m_chessboard.getSquareHeight(); // Récupérer la hauteur des cases
    
    // Convertir les coordonnées de l'échiquier en coordonnées 3D
    // L'origine est dans le coin inférieur gauche
    float xPos = (float)x * squareSize - 3.5f * squareSize; // Centré sur l'origine
    float yPos = squareHeight; // Positionner au sommet des cases
    float zPos = (float)y * squareSize - 3.5f * squareSize;
    
    return glm::vec3(xPos, yPos, zPos); // y = hauteur des cases
}

bool Renderer3D::selectPieceForView(int x, int y) {
    if (!m_isInitialized) {
        return false;
    }
    
    // Vérification des limites de l'échiquier
    if (x < 0 || x >= 8 || y < 0 || y >= 8) {
        return false;
    }
    
    glm::vec3 squarePosition = getChessBoardPosition(x, y);
    bool pieceFound = false;
    PieceColor pieceColor = PieceColor::White;
    
    // Recherche de la pièce aux coordonnées spécifiées
    auto pieces = m_pieceRenderer.getPieces();
    if (pieces.empty()) {
        return false;
    }
    
    for (const auto& piece : pieces) {
        if (piece.x == x && piece.y == y) {
            pieceFound = true;
            pieceColor = piece.color;
            break;
        }
    }
    
    if (!pieceFound) {
        return false;
    }
    
    // Mémoriser les informations de la pièce sélectionnée
    m_selectedPiecePosition = squarePosition;
    m_selectedPieceColor = pieceColor;
    m_hasPieceSelected = true;
    m_selectedPieceX = x;
    m_selectedPieceY = y;
    
    // Positionner la caméra sur la pièce
    m_camera.setPieceView(m_selectedPiecePosition, m_selectedPieceColor);
    return true;
}

void Renderer3D::updateTrackedPiece() {
    if (!m_hasPieceSelected) {
        return;
    }
    
    float squareSize = m_chessboard.getSquareSize();
    
    // Vérifier si la pièce est en cours d'animation
    if (m_pieceRenderer.isPieceAnimating(m_selectedPieceX, m_selectedPieceY)) {
        // Obtenir la position actuelle de la pièce pendant l'animation
        glm::vec3 currentPos = m_pieceRenderer.getPiecePosition(m_selectedPieceX, m_selectedPieceY, squareSize);
        
        // Mettre à jour la position de la caméra pour suivre la pièce
        m_camera.setPieceView(currentPos, m_selectedPieceColor);
    }
    else {
        // Vérifier si la pièce a changé de position après animation
        int newX = m_selectedPieceX;
        int newY = m_selectedPieceY;
        
        if (m_pieceRenderer.findNewPiecePosition(m_selectedPieceX, m_selectedPieceY, newX, newY)) {
            // La pièce a changé de position
            m_selectedPieceX = newX;
            m_selectedPieceY = newY;
            
            // Mettre à jour la position de la caméra
            m_selectedPiecePosition = getChessBoardPosition(newX, newY);
            m_camera.setPieceView(m_selectedPiecePosition, m_selectedPieceColor);
        }
    }
}

void Renderer3D::toggleCameraMode() {
    std::cout << "Tentative de basculement du mode caméra" << std::endl;
    std::cout << "Mode actuel: " << (m_camera.getCameraMode() == CameraMode::Trackball ? "Trackball" : "Pièce") << std::endl;
    std::cout << "Pièce sélectionnée: " << (m_hasPieceSelected ? "Oui" : "Non") << std::endl;
    
    if (m_camera.getCameraMode() == CameraMode::Trackball) {
        // Si on est en mode trackball et qu'on veut passer en mode pièce
        if (!m_hasPieceSelected) {
            // Si aucune pièce n'est sélectionnée, on sélectionne automatiquement le roi blanc
            if (selectPieceForView(4, 0)) { // Roi blanc (position 4,0)
                std::cout << "Sélection automatique du Roi blanc pour la vue en mode pièce" << std::endl;
            } else {
                // En cas d'échec, on essaie avec la dame blanche
                if (selectPieceForView(3, 0)) {
                    std::cout << "Sélection automatique de la Dame blanche pour la vue en mode pièce" << std::endl;
                } else {
                    // Si on ne trouve aucune pièce valide, on ne peut pas changer de mode
                    std::cout << "ERREUR: Aucune pièce disponible pour la vue en mode pièce" << std::endl;
                    return;
                }
            }
        }
        
        // Maintenant une pièce est sélectionnée, on peut changer de mode
        std::cout << "Passage en mode pièce avec position: (" << m_selectedPiecePosition.x << ", " 
                  << m_selectedPiecePosition.y << ", " << m_selectedPiecePosition.z << ")" << std::endl;
        m_camera.setPieceView(m_selectedPiecePosition, m_selectedPieceColor);
        std::cout << "Nouveau mode: " << (m_camera.getCameraMode() == CameraMode::Trackball ? "Trackball" : "Pièce") << std::endl;
    } else {
        // Revenir au mode trackball
        std::cout << "Retour au mode trackball" << std::endl;
        m_camera.toggleCameraMode();
        std::cout << "Nouveau mode: " << (m_camera.getCameraMode() == CameraMode::Trackball ? "Trackball" : "Pièce") << std::endl;
    }
}