#include "PieceRenderer.hpp"
#include <algorithm>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

PieceRenderer::PieceRenderer()
    : m_piecesLoaded(false)
{
}

PieceRenderer::~PieceRenderer()
{
    cleanup();
}

bool PieceRenderer::initialize()
{
    if (!createShader())
    {
        std::cerr << "ERROR: Failed to create piece shader" << std::endl;
        return false;
    }

    return true;
}

void PieceRenderer::update(float deltaTime)
{
    for (auto& piece : m_pieces)
    {
        if (piece.state != AnimationState::Idle)
        {
            piece.animationTime += deltaTime;
            // Animation terminée
            if (piece.animationTime >= piece.animationDuration)
            {
                piece.animationTime = piece.animationDuration;
                piece.state         = AnimationState::Idle;

                // Si la pièce était en train d'être capturée, la supprimer
                if (piece.isBeingCaptured)
                {
                    // Marquer pour suppression (sera traitée après la boucle)
                    piece.type = PieceType::None;
                }
            }
        }
    }

    // Supprimer les pièces capturées (marquées avec type None)
    m_pieces.erase(
        std::remove_if(
            m_pieces.begin(),
            m_pieces.end(),
            [](const ChessPiece& p) { return p.type == PieceType::None; }
        ),
        m_pieces.end()
    );

    // Si toutes les animations sont terminées et qu'on a un état suivant,
    // on peut passer à cet état
    if (!m_nextState.empty() && !isAnimating())
    {
        m_pieces = m_nextState;
        m_nextState.clear();
    }
}

void PieceRenderer::render(const glm::mat4& view, const glm::mat4& projection, float squareSize)
{
    if (!m_piecesLoaded || m_pieces.empty())
    {
        return;
    }
    m_pieceShader.use();

    // Passer les matrices de vue et projection
    GLuint viewPieceLoc  = glGetUniformLocation(m_pieceShader.getGLId(), "view");
    GLuint projPieceLoc  = glGetUniformLocation(m_pieceShader.getGLId(), "projection");
    GLuint modelPieceLoc = glGetUniformLocation(m_pieceShader.getGLId(), "model");
    GLuint pieceColorLoc = glGetUniformLocation(m_pieceShader.getGLId(), "pieceColor");

    glUniformMatrix4fv(viewPieceLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projPieceLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Dessiner chaque pièce
    for (const auto& piece : m_pieces)
    {
        auto it = m_pieceData.find(piece.type);
        if (it == m_pieceData.end())
        {
            continue; // Sauter si le modèle n'est pas chargé
        }

        // Définir la couleur de la pièce en fonction de PieceColor
        glm::vec3 color;
        if (piece.color == PieceColor::White)
        {
            color = glm::vec3(0.95f, 0.95f, 0.85f); // Blanc
        }
        else
        {
            color = glm::vec3(0.15f, 0.15f, 0.15f); // Noir
        }

        // Si la pièce est en train d'être capturée, modifier sa couleur
        if (piece.isBeingCaptured)
        {
            float captureProgress = piece.animationTime / piece.animationDuration;
            color                 = glm::mix(color, glm::vec3(0.8f, 0.0f, 0.0f), captureProgress);
        }

        // Passer la couleur au shader uniquement si l'uniform existe
        if (pieceColorLoc != -1)
        {
            glUniform3fv(pieceColorLoc, 1, glm::value_ptr(color));
        }

        // Calculer la matrice de modèle en tenant compte de l'animation
        glm::mat4 pieceModel = calculateAnimatedModelMatrix(piece, squareSize);

        glUniformMatrix4fv(modelPieceLoc, 1, GL_FALSE, glm::value_ptr(pieceModel));

        // Référence aux données de rendu de la pièce
        const auto& renderData = it->second;

        // Dessiner la pièce avec son VAO spécifique
        glBindVertexArray(renderData.vao);
        for (size_t i = 0; i < renderData.geometry.getMeshCount(); i++)
        {
            const auto& mesh = renderData.geometry.getMeshBuffer()[i];
            glDrawElements(GL_TRIANGLES, mesh.m_nIndexCount, GL_UNSIGNED_INT, (void*)(mesh.m_nIndexOffset * sizeof(unsigned int)));
        }
    }

    glBindVertexArray(0);
    glUseProgram(0);
}

bool PieceRenderer::loadPieceModel(PieceType type, const std::string& modelPath)
{
    std::cout << "Loading chess piece model: " << modelPath << std::endl;

    try
    {
        // Déterminer le chemin de base pour les textures
        std::string modelDir = modelPath.substr(0, modelPath.find_last_of("/\\") + 1);

        // Créer une nouvelle géométrie pour ce type de pièce
        glBurnout::Geometry pieceGeometry;

        // Charger le modèle OBJ
        if (!pieceGeometry.loadOBJ(modelPath, modelDir))
        {
            std::cerr << "ERROR: Failed to load piece model: " << modelPath << std::endl;
            return false;
        }

        /*
        std::cout << "Successfully loaded model with "
                  << pieceGeometry.getVertexCount() << " vertices, "
                  << pieceGeometry.getIndexCount() << " indices, and "
                  << pieceGeometry.getMeshCount() << " meshes." << std::endl;
        */

        // Configurer les buffers pour ce type de pièce
        setupBuffers(type, pieceGeometry);

        m_piecesLoaded = true;
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: Exception during model loading: " << e.what() << std::endl;
        return false;
    }
}

void PieceRenderer::addPiece(PieceType type, PieceColor color, int x, int y)
{
    // Vérifier que le type de pièce a un modèle chargé
    if (m_pieceData.find(type) == m_pieceData.end())
    {
        std::cerr << "WARNING: Cannot add piece of type " << static_cast<int>(type)
                  << " - model not loaded" << std::endl;
        return;
    }

    ChessPiece piece;
    piece.type  = type;
    piece.color = color;
    piece.x     = x;
    piece.y     = y;

    m_pieces.push_back(piece);
}

void PieceRenderer::clearPieces()
{
    m_pieces.clear();
}

void PieceRenderer::cleanup()
{
    // Nettoyer tous les VAOs, VBOs et EBOs pour chaque type de pièce
    for (auto& data : m_pieceData)
    {
        if (data.second.vao)
        {
            glDeleteVertexArrays(1, &data.second.vao);
        }
        if (data.second.vbo)
        {
            glDeleteBuffers(1, &data.second.vbo);
        }
        if (data.second.ebo)
        {
            glDeleteBuffers(1, &data.second.ebo);
        }
    }

    m_pieceData.clear();
    m_pieces.clear();
    m_piecesLoaded = false;
}

bool PieceRenderer::createShader()
{
    try
    {
        // Chemin vers les fichiers de shader
        std::string basePath           = "../../shaders/";
        std::string vertexShaderPath   = basePath + "piece.vs.glsl";
        std::string fragmentShaderPath = basePath + "piece.fs.glsl";

        // Vérifier si les fichiers existent
        std::ifstream testVertex(vertexShaderPath);
        std::ifstream testFragment(fragmentShaderPath);

        if (testVertex.good() && testFragment.good())
        {
            std::cout << "Shaders des pièces trouvés au chemin: " << basePath << std::endl;

            glBurnout::FilePath vsPath(vertexShaderPath);
            glBurnout::FilePath fsPath(fragmentShaderPath);

            // Charger le programme shader à partir des fichiers
            m_pieceShader = glBurnout::loadProgram(vsPath, fsPath);
            return true;
        }
        else
        {
            // Si les fichiers n'existent pas, utiliser les shaders intégrés
            std::cout << "INFO: Fichiers de shader pour les pièces non trouvés, utilisation des shaders intégrés" << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: Failed to create piece shader: " << e.what() << std::endl;
        return false;
    }
    return false;
}

void PieceRenderer::setupBuffers(PieceType type, const glBurnout::Geometry& geometry)
{
    PieceRenderData& renderData = m_pieceData[type];

    // Copier la géométrie
    renderData.geometry = geometry;

    // Générer les buffers OpenGL
    glGenVertexArrays(1, &renderData.vao);
    glGenBuffers(1, &renderData.vbo);
    glGenBuffers(1, &renderData.ebo);

    glBindVertexArray(renderData.vao);

    // Charger les données de vertex
    glBindBuffer(GL_ARRAY_BUFFER, renderData.vbo);
    glBufferData(GL_ARRAY_BUFFER, geometry.getVertexCount() * sizeof(glBurnout::Geometry::Vertex), geometry.getVertexBuffer(), GL_STATIC_DRAW);

    // Charger les indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderData.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, geometry.getIndexCount() * sizeof(unsigned int), geometry.getIndexBuffer(), GL_STATIC_DRAW);

    // Configurer les attributs de vertex
    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glBurnout::Geometry::Vertex), (void*)offsetof(glBurnout::Geometry::Vertex, m_Position));

    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glBurnout::Geometry::Vertex), (void*)offsetof(glBurnout::Geometry::Vertex, m_Normal));

    // TexCoord
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glBurnout::Geometry::Vertex), (void*)offsetof(glBurnout::Geometry::Vertex, m_TexCoords));

    glBindVertexArray(0);
}

glm::vec3 PieceRenderer::calculateChessPosition(int x, int y, float squareSize) const
{
    const float pieceOffset = squareSize * 0.5f;
    const float pieceHeight = 0.1f;

    return glm::vec3(
        -4 * squareSize + x * squareSize + pieceOffset,
        pieceHeight,
        -4 * squareSize + y * squareSize + pieceOffset
    );
}

glm::mat4 PieceRenderer::calculateAnimatedModelMatrix(const ChessPiece& piece, float squareSize) const
{
    glm::mat4 model = glm::mat4(1.0f);

    const float pieceOffset = squareSize * 0.5f;
    const float pieceScale  = 7.0f;
    const float pieceHeight = 0.1f;

    if (piece.state == AnimationState::Idle)
    {
        // Positionnement normal de la pièce
        model = glm::translate(model, glm::vec3(-4 * squareSize + piece.x * squareSize + pieceOffset, pieceHeight, -4 * squareSize + piece.y * squareSize + pieceOffset));
    }
    else
    {
        // Animation en cours
        float t = piece.animationTime / piece.animationDuration;

        // Ajout d'une courbe d'accélération/décélération
        float smoothT = t * t * (3.0f - 2.0f * t);

        if (piece.state == AnimationState::Moving || piece.state == AnimationState::Capturing)
        {
            // Interpolation linéaire entre les positions de départ et d'arrivée
            glm::vec3 currentPos;

            if (piece.isBeingCaptured)
            {
                // Pour les pièces capturées, on les fait tomber de l'échiquier
                currentPos = glm::mix(
                    piece.startPosition,
                    glm::vec3(piece.startPosition.x, 0.5f, piece.startPosition.z),
                    smoothT
                );

                // Ajouter une rotation lors de la capture
                // TODO :  VOIR POUR AJOUTER UNE LOIS DE PROBA POUR PLUS DE RANDOM...
                float captureRotation = smoothT * 90.0f;
                model                 = glm::translate(model, currentPos);
                model                 = glm::rotate(model, glm::radians(captureRotation), glm::vec3(1.0f, 0.0f, 0.0f));
            }
            else
            {
                // Un petit saut
                float jumpFactor = 4.0f * smoothT * (1.0f - smoothT);

                currentPos = glm::mix(piece.startPosition, piece.targetPosition, smoothT);
                currentPos.y += piece.jumpHeight * jumpFactor;

                model = glm::translate(model, currentPos);
            }
        }
    }

    // Rotation pour que les pièces noires soient orientées correctement
    if (piece.color == PieceColor::Black)
    {
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    // TODO : AJOUTER UNE ROTATION ALEATOIRE POUR CHAQUE PIECE ?

    // Mise à l'échelle pour toutes les pièces
    model = glm::scale(model, glm::vec3(pieceScale));

    return model;
}

void PieceRenderer::startPieceMovement(ChessPiece& piece, int targetX, int targetY, float duration)
{
    piece.state             = AnimationState::Moving;
    piece.animationTime     = 0.0f;
    piece.animationDuration = duration;

    // position actuelle comme position de départ pour l'animation
    const float squareSize = 1.0f;
    piece.startPosition    = calculateChessPosition(piece.x, piece.y, squareSize);
    piece.targetPosition   = calculateChessPosition(targetX, targetY, squareSize);

    // Calculer la hauteur du saut en fonction de la distance
    float distance   = std::sqrt(pow(targetX - piece.x, 2) + pow(targetY - piece.y, 2));
    piece.jumpHeight = 0.3f * distance;

    // Mettre à jour les coordonnées de la pièce
    piece.x = targetX;
    piece.y = targetY;
}

void PieceRenderer::animateTransition(const std::vector<ChessPiece>& newState, float duration)
{
    // Si pas de pièces actuelles, adopter directement le nouvel état
    if (m_pieces.empty())
    {
        m_pieces = newState;
        return;
    }

    bool                    hasChanges      = false;
    std::vector<ChessPiece> mutableNewState = newState;
    std::vector<bool>       oldPieceMatched(m_pieces.size(), false);
    std::vector<bool>       newPieceMatched(mutableNewState.size(), false);

    // Identifier d'abord les pièces qui n'ont pas bougé
    for (size_t i = 0; i < m_pieces.size(); ++i)
    {
        for (size_t j = 0; j < mutableNewState.size(); ++j)
        {
            if (!newPieceMatched[j] && m_pieces[i].type == mutableNewState[j].type && m_pieces[i].color == mutableNewState[j].color && m_pieces[i].x == mutableNewState[j].x && m_pieces[i].y == mutableNewState[j].y)
            {
                oldPieceMatched[i] = true;
                newPieceMatched[j] = true;
                break;
            }
        }
    }

    // Déplacer les pièces qui ont changé de position
    for (size_t i = 0; i < m_pieces.size(); ++i)
    {
        if (oldPieceMatched[i])
            continue;

        for (size_t j = 0; j < mutableNewState.size(); ++j)
        {
            if (newPieceMatched[j])
                continue;

            // Même type et couleur mais position différente = mouvement
            if (m_pieces[i].type == mutableNewState[j].type && m_pieces[i].color == mutableNewState[j].color)
            {
                ChessPiece&       piece  = m_pieces[i];
                const ChessPiece& target = mutableNewState[j];

                startPieceMovement(piece, target.x, target.y, duration);
                hasChanges = true;

                oldPieceMatched[i] = true;
                newPieceMatched[j] = true;
                break;
            }
        }
    }

    // Les pièces non matchées dans l'ancien état ont été capturées
    for (size_t i = 0; i < m_pieces.size(); ++i)
    {
        if (!oldPieceMatched[i])
        {
            // Animer la capture de la pièce
            m_pieces[i].state             = AnimationState::Capturing;
            m_pieces[i].isBeingCaptured   = true;
            m_pieces[i].animationTime     = 0.0f;
            m_pieces[i].animationDuration = duration * 0.8f; // Un peu plus rapide
            m_pieces[i].startPosition     = calculateChessPosition(m_pieces[i].x, m_pieces[i].y, 1.0f);
            hasChanges                    = true;
        }
    }

    // Les pièces non matchées dans le nouvel état sont nouvelles (promotion)
    for (size_t j = 0; j < mutableNewState.size(); ++j)
    {
        if (!newPieceMatched[j])
        {
            addPiece(mutableNewState[j].type, mutableNewState[j].color, mutableNewState[j].x, mutableNewState[j].y);
            hasChanges = true;
        }
    }

    // Stocker le prochain état si des changements ont été détectés
    if (hasChanges)
    {
        m_nextState = mutableNewState;
    }
    else
    {
        // Sinon appliquer directement le nouvel état
        m_pieces = mutableNewState;
    }
}

bool PieceRenderer::isAnimating() const
{
    for (const auto& piece : m_pieces)
    {
        if (piece.state != AnimationState::Idle)
        {
            return true;
        }
    }
    return false;
}

glm::vec3 PieceRenderer::getPiecePosition(int x, int y, float squareSize) const
{
    // Chercher la pièce aux coordonnées données
    for (const auto& piece : m_pieces)
    {
        if (piece.x == x && piece.y == y)
        {
            // Si la pièce est en animation, calculer sa position actuelle
            if (piece.state != AnimationState::Idle)
            {
                float t       = piece.animationTime / piece.animationDuration;
                float smoothT = t * t * (3.0f - 2.0f * t); // Courbe d'accélération/décélération

                if (piece.isBeingCaptured)
                {
                    // Pièce en cours de capture - retourner la position de départ
                    return piece.startPosition;
                }
                else
                {
                    // Pièce en déplacement normal
                    float     jumpFactor = 4.0f * smoothT * (1.0f - smoothT);
                    glm::vec3 currentPos = glm::mix(piece.startPosition, piece.targetPosition, smoothT);
                    currentPos.y += piece.jumpHeight * jumpFactor;
                    return currentPos;
                }
            }
            else
            {
                // Si la pièce est immobile, calculer sa position normale sur l'échiquier
                return calculateChessPosition(piece.x, piece.y, squareSize);
            }
        }
    }

    // Si la pièce n'est pas trouvée, vérifier l'état suivant (si en animation)
    if (!m_nextState.empty())
    {
        for (const auto& piece : m_nextState)
        {
            if (piece.x == x && piece.y == y)
            {
                return calculateChessPosition(piece.x, piece.y, squareSize);
            }
        }
    }

    // Si la pièce n'est pas trouvée, retourner une position par défaut
    return calculateChessPosition(x, y, squareSize);
}

bool PieceRenderer::isPieceAnimating(int x, int y) const
{
    for (const auto& piece : m_pieces)
    {
        if (piece.x == x && piece.y == y && piece.state != AnimationState::Idle)
        {
            return true;
        }
    }
    return false;
}

bool PieceRenderer::findNewPiecePosition(int oldX, int oldY, int& newX, int& newY) const
{
    // Cette méthode permet de trouver la nouvelle position d'une pièce après une animation
    // Pour chaque pièce qui est d'une position différente de sa position initiale
    for (const auto& piece : m_pieces)
    {
        // Vérifier si cette pièce est en cours d'animation
        if (piece.state != AnimationState::Idle && !piece.isBeingCaptured)
        {
            // Calculer la position du début de l'animation
            glm::vec3 startPos = piece.startPosition;
            int       startX   = (int)roundf((startPos.x + 4.0f) - 0.5f);
            int       startY   = (int)roundf((startPos.z + 4.0f) - 0.5f);

            // Si c'est notre pièce d'origine
            if (startX == oldX && startY == oldY)
            {
                // Retourner la nouvelle position
                newX = piece.x;
                newY = piece.y;
                return true;
            }
        }
    }
    // Vérifier dans l'état suivant
    if (!m_nextState.empty())
    {
        for (const auto& piece : m_nextState)
        {
            if (piece.x == oldX && piece.y == oldY)
            {
                newX = piece.x;
                newY = piece.y;
                return true;
            }
        }
    }
    // Si aucune correspondance n'est trouvée, la pièce peut avoir été capturée
    return false;
}
