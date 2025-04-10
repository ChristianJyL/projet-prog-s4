#include "Board.hpp"
#include <imgui.h>
#include <array>
#include <iostream>
#include <string>
#include <vector>
#include "../3Dengine/Renderer3D.hpp"
#include "GameMode/ClassicChess.hpp"
#include "GameMode/DrunkChess.hpp"

Board::Board()
    : m_currentGameMode(std::make_unique<ClassicChessMode>()), // par défaut, le mode classique
      m_winner(PieceColor::White) // Initialisation du vainqueur
{
}

void Board::setGameMode(std::unique_ptr<GameMode> mode)
{
    m_currentGameMode = std::move(mode);
    m_gameOver        = false;
    m_turn            = PieceColor::White;
}

void Board::initializeBoard(Renderer3D* renderer)
{
    if (renderer) {
        m_renderer3D = renderer;
    }
    // on délègue l'initialisation à la classe de mode de jeu actuelle
    if (m_currentGameMode)
    {
        m_currentGameMode->initializeBoard(m_list);
    }
    m_lastDoublePawnMove.reset();
    m_renderer3D->updatePiecesFromBoard(*this);
}

//getter pour le vector
Piece Board::get(Position pos) const
{
    return m_list.at(pos.x + pos.y * 8);
}

//Setter pour le vector
void Board::set(Position pos, Piece piece)
{
    m_list.at(pos.x + pos.y * 8) = piece;
}

//Méthode pour déplacer une pièce dans le vector
void Board::move(Position from, Position to)
{
    set(to, get(from));
    set(from, {PieceType::None, PieceColor::White});

    m_renderer3D->updatePiecesFromBoard(*this);
}

ImVec4 Board::getPieceColor(Piece piece) const
{
    return (piece.color == PieceColor::White) ? ImVec4{1.0f, 1.0f, 1.0f, 1.0f} : ImVec4{0.0f, 0.0f, 0.0f, 1.0f};
}

void Board::drawTile(int index, bool pairLine, ImVec2& outCursorPos)
{
    Position pos = {index % 8, index / 8};

    ImVec4 tileColor;
    if (m_currentGameMode)
    {
        //on délègue la couleur de la case à la classe de mode de jeu actuelle
        tileColor = m_currentGameMode->getTileColor(pairLine, index, pos);
    }
    ImGui::PushStyleColor(ImGuiCol_Button, tileColor);

    // Récupération de la pièce
    Piece       piece      = m_list.at(index);
    std::string label      = std::string(1, piece.toChar());
    ImVec4      pieceColor = getPieceColor(piece);

    ImGui::PushID(index);
    ImGui::PushStyleColor(ImGuiCol_Text, pieceColor);

    // Stocker la position actuelle du curseur avant d'afficher le bouton
    outCursorPos = ImGui::GetCursorScreenPos();
    // Taille du bouton
    ImVec2 buttonSize = ImVec2{50.f, 50.f};

    // Dessiner le bouton de la case
    ImGui::Button(label.c_str(), buttonSize);

    // Gestion des interactions souris
    handleMouseInteraction(index);

    ImGui::PopStyleColor();
    ImGui::PopID();
    ImGui::PopStyleColor();

    // Dessiner les effets spécifiques au mode
    if (piece.type != PieceType::None)
    {
        m_currentGameMode->drawTileEffect(pos, outCursorPos, piece);
    }
}

void Board::drawPossibleMoves(Position pos, ImVec2 cursorPos)
{
    if (!m_selectedPiece)
        return;

    Piece selectedPiece = get(*m_selectedPiece);
    if (selectedPiece.type == PieceType::None)
        return;

    for (const Position& move : getValidMoves(*m_selectedPiece))
    {
        if (move.x == pos.x && move.y == pos.y)
        {
            ImGui::GetWindowDrawList()->AddCircleFilled(
                ImVec2(cursorPos.x + 25, cursorPos.y + 25), // Centre du bouton
                10.0f,                                      // Taille du cercle
                IM_COL32(0, 255, 0, 150)                    // Vert
            );
        }
    }
}

void Board::drawBoard()
{
    ImGui::Begin("Chess");
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    bool pairLine = true;

    for (int i = 0; i < m_list.size(); ++i)
    {
        if (i % 8 == 0)
        {
            pairLine = !pairLine;
        }
        else
        {
            ImGui::SameLine();
        }

        Position pos = {i % 8, i / 8};
        ImVec2   cursorPos;
        drawTile(i, pairLine, cursorPos); 
        drawPossibleMoves(pos, cursorPos); 
    }

    ImGui::PopStyleVar();
    handlePawnPromotion();
    ImGui::End();
}

void Board::handleMouseInteraction(int index)
{
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        handleClick(Position{index % 8, index / 8});
    }
    else if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
    {
        m_selectedPiece.reset(); 
    }
}

void Board::handleClick(Position pos)
{
    if (m_promotionInProgress)
        return;

    Piece piece = get(pos);
    // Si aucune pièce n'est sélectionnée, on essaye de sélectionner
    if (!m_selectedPiece)
    {
        selectPiece(pos);
    }
    // Si une pièce est déjà sélectionnée
    else
    {
        Position from = *m_selectedPiece;
        // Si on clique sur une pièce du même joueur -> on sélectionne celle-ci à la place
        if (piece.type != PieceType::None && ((m_turn == PieceColor::White && piece.color == PieceColor::White) || (m_turn == PieceColor::Black && piece.color == PieceColor::Black)))
        {
            selectPiece(pos);
        }
        // Sinon, on tente un déplacement (vide ou capture)
        else
        {
            movePiece(pos);
        }
    }
}

void Board::selectPiece(Position pos)
{
    Piece selectedPiece = get(pos);

    // Vérifier que la pièce appartient au joueur actif
    if (selectedPiece.type != PieceType::None && selectedPiece.color == m_turn)
    {
        m_selectedPiece = pos;
        // la cam sur la pièce sélectionnée
        syncCameraWithSelection();
    }
    else
    {
        // Pièce de l'adversaire ou case vide, on ne fait rien
        m_selectedPiece.reset();
    }
}

void Board::syncCameraWithSelection()
{
    if (m_renderer3D && m_selectedPiece.has_value())
    {
        Position pos = m_selectedPiece.value();
        // Ne synchroniser la caméra que si elle est déjà en mode pièce
        if (m_renderer3D->getCameraMode() == CameraMode::Piece) {
            m_renderer3D->selectPieceForView(pos.x, pos.y);
        }
    }
}

//Vérifier si une pièce bloque le passage
bool Board::isPathClear(Position from, Position to) const
{
    int dx = to.x - from.x;
    int dy = to.y - from.y;

    int stepX = (dx == 0) ? 0 : (dx > 0 ? 1 : -1);
    int stepY = (dy == 0) ? 0 : (dy > 0 ? 1 : -1);

    Position checkPos = from;
    checkPos.x += stepX;
    checkPos.y += stepY;

    while (checkPos.x != to.x || checkPos.y != to.y)
    {
        if (!checkPos.isValid() || get(checkPos).type != PieceType::None)
        {
            return false; // Une pièce bloque le passage
        }
        checkPos.x += stepX;
        checkPos.y += stepY;
    }

    return true;
}

bool Board::isEnPassantCapture(Position from, Position to) const
{
    if (!m_lastDoublePawnMove)
        return false;

    Piece piece = get(from);

    // Vérifier si c'est un pion
    if (piece.type != PieceType::Pawn)
        return false;

    // Vérifier si le mouvement est une capture en diagonale
    int dx        = to.x - from.x;
    int dy        = to.y - from.y;
    int direction = (piece.color == PieceColor::White) ? 1 : -1;

    if (abs(dx) != 1 || dy != direction)
        return false;

    // Vérifier si la position cible est directement derrière le pion qui a fait un double mouvement
    Position pawnPos   = *m_lastDoublePawnMove;
    int      expectedY = (piece.color == PieceColor::White) ? 4 : 3; // Le pion capturé doit être sur la 5e ou 4e rangée

    return to.x == pawnPos.x && from.y == expectedY && pawnPos.y == from.y;
}

bool Board::isPawnPromotion(Position from, Position to, Piece piece) const
{
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

bool Board::isGameOver() const
{
    return m_gameOver;
}

void Board::movePiece(Position pos)
{
    if (!m_selectedPiece)
        return;

    Position from        = *m_selectedPiece;
    Piece    piece       = get(from);
    Piece    targetPiece = get(pos);

    // Vérifier d'abord si le mouvement est valide (selon les règles classico)
    if (!m_currentGameMode->isValidMove(m_list, from, pos, piece))
    {
        m_selectedPiece.reset();
        return;
    }
        
    bool isPawnDoubleMove = false;

    //Maintenant, on vérifie les cas spécials
    if (piece.type == PieceType::Pawn && abs(pos.x - from.x) == 1)
    {
        if (isEnPassantCapture(from, pos))
        {
            // Déterminer la position du pion à capturer
            Position capturedPawnPos = {pos.x, from.y};
            // Exécuter un mouvement en passant
            move(from, pos);
            set(capturedPawnPos, {PieceType::None, PieceColor::White});
        }
        // Capture normale: un pion ne peut se déplacer en diagonale que s'il y a une pièce ennemie à capturer
        else if (targetPiece.type == PieceType::None)
        {
            m_selectedPiece.reset(); // Annuler la sélection
            return;
        }
        else
        {
            // Déléguer au mode de jeu
            m_currentGameMode->executeMove(m_list, from, pos);
        }
    }
    else
    {
        // Déléguer au mode de jeu pour les autres types de mouvements
        m_currentGameMode->executeMove(m_list, from, pos);
    }

    // Vérifier s'il s'agit d'un mouvement de deux cases pour un pion
    if (piece.type == PieceType::Pawn)
    {
        int dy = pos.y - from.y;
        if (abs(dy) == 2)
        {
            m_lastDoublePawnMove = pos;
            isPawnDoubleMove = true;
        }
        else
        {
            m_lastDoublePawnMove.reset(); 
        }
    }
    else
    {
        m_lastDoublePawnMove.reset();
    }

    m_renderer3D->updatePiecesFromBoard(*this);

    if (targetPiece.type == PieceType::King){
        m_gameOver = true;
        m_winner = piece.color; 
        m_promotionInProgress = false;
    }

    // Gérer la promotion de pion seulement si le jeu n'est pas terminé
    if (isPawnPromotion(from, pos, piece) && !m_gameOver)
    {
        m_promotionInProgress = true;
        m_promotionPosition   = pos;
        m_promotionColor      = piece.color;
    }
    else if (!m_gameOver)
    {
        nextTurn();
    }

    m_selectedPiece.reset(); 
}

void Board::executeMove(Position from, Position to)
{
    Piece piece = get(from);
    bool isEnPassant = isEnPassantCapture(from, to);
    Position capturedPawnPos;

    if (isEnPassant)
    {
        capturedPawnPos = {to.x, from.y}; // Le pion à capturer est sur la même rangée que notre pion
    }

    // Exécuter le mouvement
    move(from, to);

    // Si c'était une capture en passant, enlever le pion capturé
    if (isEnPassant)
    {
        set(capturedPawnPos, {PieceType::None, PieceColor::White});
    }

    // Mettre à jour m_lastDoublePawnMove si c'était un double mouvement de pion
    if (piece.type == PieceType::Pawn)
    {
        int dy = to.y - from.y;
        if (abs(dy) == 2)
        {
            m_lastDoublePawnMove = to;
        }
        else
        {
            m_lastDoublePawnMove.reset(); 
        }
    }
    else
    {
        m_lastDoublePawnMove.reset(); 
    }

    m_renderer3D->updatePiecesFromBoard(*this);
}

void Board::nextTurn()
{
    m_turn = (m_turn == PieceColor::White) ? PieceColor::Black : PieceColor::White;

    if (m_currentGameMode)
    {
        m_currentGameMode->updatePerTurn(m_list, m_turn);
    }
}

void Board::handlePawnPromotion()
{
    // Ne pas ouvrir la fenêtre de promotion si le jeu est terminé
    if (!m_promotionInProgress || m_gameOver)
        return;

    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Promotion de pion", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Choisissez une pièce pour la promotion :\n\n");

        std::array<std::pair<PieceType, const char*>, 4> options = {
            std::make_pair(PieceType::Queen, "Reine"),
            std::make_pair(PieceType::Rook, "Tour"),
            std::make_pair(PieceType::Bishop, "Fou"),
            std::make_pair(PieceType::Knight, "Cavalier")
        };

        for (auto& [type, name] : options)
        {
            if (ImGui::Button(name, ImVec2(100, 40)))
            {
                set(m_promotionPosition, {type, m_promotionColor});

                m_renderer3D->updatePiecesFromBoard(*this);

                m_promotionInProgress = false;
                ImGui::CloseCurrentPopup();

                if (!m_gameOver) {
                    nextTurn();
                }
            }
        }

        ImGui::EndPopup();
    }

    // Ouvrir la fenêtre modale si une promotion est en cours et que le jeu n'est pas terminé
    if (m_promotionInProgress && !m_gameOver)
    {
        ImGui::OpenPopup("Promotion de pion");
    }
}

//Uniquement visuel
//Pour avoir la liste des positions valides pour un mouvement
std::vector<Position> Board::getValidMoves(Position from) const
{
    std::vector<Position> moves;
    Piece                 piece = get(from);
    if (piece.type == PieceType::None)
        return moves;

    // Vérifier spécifiquement la capture en passant pour les pions
    if (piece.type == PieceType::Pawn && m_lastDoublePawnMove)
    {
        Position pawnPos   = *m_lastDoublePawnMove;
        int      direction = (piece.color == PieceColor::White) ? 1 : -1;
        int      expectedY = (piece.color == PieceColor::White) ? 4 : 3;

        // Si le pion est adjacent au pion qui a fait un double mouvement et sur la bonne rangée
        if (from.y == expectedY && abs(from.x - pawnPos.x) == 1 && pawnPos.y == from.y)
        {
            // Vérifier que c'est bien un pion ennemi
            Piece possiblePawn = get(pawnPos);
            if (possiblePawn.type == PieceType::Pawn && possiblePawn.color != piece.color)
            {
                // Ajouter la position de capture en passant
                Position enPassantPos = {pawnPos.x, from.y + direction};
                moves.push_back(enPassantPos);
            }
        }
    }

    // Ajouter le reste des mouvements valides comme avant
    for (int x = 0; x < 8; ++x)
    {
        for (int y = 0; y < 8; ++y)
        {
            Position to = {x, y};

            // Vérification spéciale pour les pions qui se déplacent en diagonale
            if (piece.type == PieceType::Pawn && abs(to.x - from.x) == 1 )
            {
                int dy        = to.y - from.y;
                int direction = (piece.color == PieceColor::White) ? 1 : -1;

                // Vérifier que c'est bien un mouvement diagonal vers l'avant
                if (dy != direction)
                {
                    continue;
                }

                Piece targetPiece = get(to);
                // Un pion ne peut aller en diagonale que s'il y a une pièce ennemie
                if (targetPiece.type == PieceType::None)
                {
                    continue; // Pas de pièce à capturer, on ne peut pas aller en diagonale ici
                }
                else if (targetPiece.color == piece.color)
                {
                    continue; // Ne peut pas capturer sa propre couleur
                }
            }

            //si il y a un ennemi devant un pion qui avance de deux cases
            if (piece.type == PieceType::Pawn && abs(to.x - from.x) == 0 && abs(to.y - from.y) == 2)
            {
                Position middlePos = {from.x, from.y + ((piece.color == PieceColor::White) ? 1 : -1)};
                Piece    middlePiece = get(middlePos);
                if (middlePiece.type != PieceType::None)
                {
                    continue; // Une pièce bloque le passage
                }
            }

            if (!piece.isMoveValid(from, to))
                continue;

            Piece targetPiece = get(to);

            // Vérifier les obstacles pour la Tour, le Fou et la Reine
            if ((piece.type == PieceType::Rook || piece.type == PieceType::Bishop || piece.type == PieceType::Queen) && !isPathClear(from, to))
            {
                continue; // Bloqué par une autre pièce
            }

            // Empêcher de se déplacer sur une pièce alliée
            if (targetPiece.type == PieceType::None || targetPiece.color != piece.color)
            {
                moves.push_back(to);
            }
        }
    }
    return moves;
}

std::string Board::getCurrentModeName() const
{
    if (!m_currentGameMode)
    {
        return "Aucun";
    }

    return m_currentGameMode->getModeName();
}
    

PieceColor Board::getWinner() const
{
    return m_winner;
}
