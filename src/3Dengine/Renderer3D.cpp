#include "Renderer3D.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>

Renderer3D::Renderer3D() 
    : m_skybox(nullptr), m_chessboard(nullptr), m_pieceRenderer(nullptr), m_isInitialized(false)
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
    // Créer et initialiser la skybox
    m_skybox = new SkyBox();
    if (!m_skybox->initialize()) {
        std::cerr << "ERROR::RENDERER3D::SKYBOX_INITIALIZATION_FAILED" << std::endl;
        return false;
    }
    std::cout << "Skybox created successfully." << std::endl;
    return true;
}

bool Renderer3D::initializeChessboard() {
    // Créer et initialiser l'échiquier 3D
    m_chessboard = new Chessboard(8); // Échiquier 8x8 standard
    if (!m_chessboard->initialize()) {
        std::cerr << "ERROR::RENDERER3D::CHESSBOARD_INITIALIZATION_FAILED" << std::endl;
        return false;
    }
    std::cout << "Chessboard created successfully." << std::endl;
    return true;
}

bool Renderer3D::initializePieces() {
    // Créer et initialiser le gestionnaire de pièces
    m_pieceRenderer = new PieceRenderer();
    if (!m_pieceRenderer->initialize()) {
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
        
        if (m_pieceRenderer->loadPieceModel(pieceInfo.type, pieceInfo.path)) {
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
        m_pieceRenderer->clearPieces();
        
        // Placer les pièces blanches
        // Pions (rang 1)
        for (int i = 0; i < 8; i++) {
            m_pieceRenderer->addPiece(PieceType::Pawn, PieceColor::White, i, 1);
        }
        
        // Tours (coins)
        m_pieceRenderer->addPiece(PieceType::Rook, PieceColor::White, 0, 0);
        m_pieceRenderer->addPiece(PieceType::Rook, PieceColor::White, 7, 0);
        
        // Cavaliers (à côté des tours)
        m_pieceRenderer->addPiece(PieceType::Knight, PieceColor::White, 1, 0);
        m_pieceRenderer->addPiece(PieceType::Knight, PieceColor::White, 6, 0);
        
        // Fous (à côté des cavaliers)
        m_pieceRenderer->addPiece(PieceType::Bishop, PieceColor::White, 2, 0);
        m_pieceRenderer->addPiece(PieceType::Bishop, PieceColor::White, 5, 0);
        
        // Reine (à gauche du roi)
        m_pieceRenderer->addPiece(PieceType::Queen, PieceColor::White, 3, 0);
        
        // Roi (au centre)
        m_pieceRenderer->addPiece(PieceType::King, PieceColor::White, 4, 0);
        
        // Placer les pièces noires
        // Pions (rang 6)
        for (int i = 0; i < 8; i++) {
            m_pieceRenderer->addPiece(PieceType::Pawn, PieceColor::Black, i, 6);
        }
        
        // Tours (coins)
        m_pieceRenderer->addPiece(PieceType::Rook, PieceColor::Black, 0, 7);
        m_pieceRenderer->addPiece(PieceType::Rook, PieceColor::Black, 7, 7);
        
        // Cavaliers (à côté des tours)
        m_pieceRenderer->addPiece(PieceType::Knight, PieceColor::Black, 1, 7);
        m_pieceRenderer->addPiece(PieceType::Knight, PieceColor::Black, 6, 7);
        
        // Fous (à côté des cavaliers)
        m_pieceRenderer->addPiece(PieceType::Bishop, PieceColor::Black, 2, 7);
        m_pieceRenderer->addPiece(PieceType::Bishop, PieceColor::Black, 5, 7);
        
        // Reine (à gauche du roi)
        m_pieceRenderer->addPiece(PieceType::Queen, PieceColor::Black, 3, 7);
        
        // Roi (au centre)
        m_pieceRenderer->addPiece(PieceType::King, PieceColor::Black, 4, 7);
    }
    
    std::cout << "Piece renderer created successfully." << std::endl;
    return true;
}

bool Renderer3D::isInitialized() const {
    return m_isInitialized;
}

void Renderer3D::update(float deltaTime) {
    m_camera.update(deltaTime);
}

void Renderer3D::cleanup() {
    if (m_skybox) {
        delete m_skybox;
        m_skybox = nullptr;
    }
    
    if (m_chessboard) {
        delete m_chessboard;
        m_chessboard = nullptr;
    }
    
    if (m_pieceRenderer) {
        delete m_pieceRenderer;
        m_pieceRenderer = nullptr;
    }
    
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
    
    // Ordre de rendu: d'abord l'échiquier, puis les pièces, puis la skybox
    
    // Rendre l'échiquier 3D
    if (m_chessboard) {
        std::cout << "Rendering chessboard..." << std::endl;
        m_chessboard->render(viewMatrix, projectionMatrix);
    } else {
        std::cerr << "WARNING: Chessboard is null in render() method" << std::endl;
    }
    
    // Rendre les pièces d'échecs
    if (m_pieceRenderer && m_chessboard) {
        std::cout << "Rendering chess pieces..." << std::endl;
        m_pieceRenderer->render(viewMatrix, projectionMatrix, m_chessboard->getSquareSize());
    } else {
        std::cerr << "WARNING: PieceRenderer or Chessboard is null in render() method" << std::endl;
    }
    
    // Rendre la skybox en dernier (enveloppe tout)
    if (m_skybox) {
        m_skybox->Draw(viewMatrix, projectionMatrix);
    } else {
        std::cerr << "WARNING: Skybox is null in render() method" << std::endl;
    }
    
    // Vérifier les erreurs OpenGL après le rendu complet
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error after rendering: 0x" << std::hex << err << std::dec << std::endl;
    }
}

void Renderer3D::updatePiecesFromBoard(const Board& board) {
    if (!m_isInitialized || !m_pieceRenderer) {
        std::cerr << "ERREUR: Renderer3D non initialisé ou PieceRenderer non disponible" << std::endl;
        return;
    }
    
    std::cout << "Mise à jour des pièces 3D depuis l'état du plateau 2D" << std::endl;
    
    // Récupérer l'état complet de l'échiquier
    const std::vector<Piece>& boardState = board.getBoardState();
    
    // Effacer toutes les pièces actuelles du renderer
    m_pieceRenderer->clearPieces();
    
    // Compteur de pièces ajoutées pour debug
    int pieceCount = 0;
    
    // Parcourir toutes les positions de l'échiquier
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            // Index dans le vecteur linéaire des pièces
            int index = x + y * 8;
            
            // Récupérer la pièce à cette position
            Piece piece = boardState[index];
            
            // Si la case n'est pas vide, ajouter la pièce au renderer
            if (piece.type != PieceType::None) {
                m_pieceRenderer->addPiece(piece.type, piece.color, x, y);
                pieceCount++;
                std::cout << "Ajout d'une pièce de type " << static_cast<int>(piece.type) 
                          << " en position (" << x << "," << y << ")" << std::endl;
            }
        }
    }
    
    std::cout << "Total de " << pieceCount << " pièces ajoutées au rendu 3D" << std::endl;
}

glm::vec3 Renderer3D::getChessBoardPosition(int x, int y) const {
    if (!m_chessboard) {
        return glm::vec3(0.0f); // Position par défaut si l'échiquier n'est pas initialisé
    }
    
    float squareSize = m_chessboard->getSquareSize();
    float squareHeight = m_chessboard->getSquareHeight(); // Récupérer la hauteur des cases
    
    // Convertir les coordonnées de l'échiquier en coordonnées 3D
    // L'origine est dans le coin inférieur gauche
    float xPos = (float)x * squareSize - 3.5f * squareSize; // Centré sur l'origine
    float yPos = squareHeight; // Positionner au sommet des cases
    float zPos = (float)y * squareSize - 3.5f * squareSize;
    
    return glm::vec3(xPos, yPos, zPos); // y = hauteur des cases
}

