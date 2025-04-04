#include "Board.hpp"
#include <imgui.h>
#include <array>
#include <iostream>
#include <string>
#include <vector>

constexpr ImVec4 COLOR_DARK_GREEN = ImVec4{0.0f, 0.39f, 0.0f, 1.0f}; // utiliser enum ?
constexpr ImVec4 COLOR_BEIGE      = ImVec4{0.96f, 0.87f, 0.70f, 1.0f};

void Board::initializeBoard()
{
    // Initialisation des pièces blanches
    std::array<PieceType, 8> Pieces = {PieceType::Rook, PieceType::Knight, PieceType::Bishop, PieceType::Queen, PieceType::King, PieceType::Bishop, PieceType::Knight, PieceType::Rook};
    for (int i = 0; i < 8; ++i)
    {
        m_list[i]     = {Pieces[i], PieceColor::White};
        m_list[i + 8] = {PieceType::Pawn, PieceColor::White};
    }
    // Initialisation des pièces noires
    for (int i = 0; i < 8; ++i)
    {
        m_list[56 + i] = {Pieces[i], PieceColor::Black};
        m_list[48 + i] = {PieceType::Pawn, PieceColor::Black};
    }

    // Réinitialiser le suivi du dernier mouvement de pion double
    m_lastDoublePawnMove.reset();
}

Piece Board::get(Position pos) const
{
    return m_list.at(pos.x + pos.y * 8);
}

void Board::set(Position pos, Piece piece)
{
    m_list.at(pos.x + pos.y * 8) = piece;
}

void Board::move(Position from, Position to)
{
    set(to, get(from));
    set(from, {PieceType::None, PieceColor::White});
}

ImVec4 Board::getTileColor(bool isPairLine, int index) const
{
    return ((isPairLine && index % 2 == 0) || (!isPairLine && index % 2 != 0)) ? COLOR_DARK_GREEN : COLOR_BEIGE;
}

ImVec4 Board::getPieceColor(Piece piece) const
{
    return (piece.color == PieceColor::White) ? ImVec4{1.0f, 1.0f, 1.0f, 1.0f} : ImVec4{0.0f, 0.0f, 0.0f, 1.0f};
}

void Board::drawTile(int index, bool pairLine, ImVec2& outCursorPos)
{
    Position pos = {index % 8, index / 8};

    // Définition de la couleur de la case
    ImVec4 tileColor = getTileColor(pairLine, index);
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
}

void Board::drawPossibleMoves(Position pos, ImVec2 cursorPos)
{
    if (!m_selectedPiece)
        return;

    Piece selectedPiece = get(*m_selectedPiece);
    if (selectedPiece.type == PieceType::None)
        return;

    // Vérifier si la case est un déplacement valide
    for (const Position& move : getValidMoves(*m_selectedPiece))
    {
        if (move.x == pos.x && move.y == pos.y)
        {
            ImGui::GetWindowDrawList()->AddCircleFilled(
                ImVec2(cursorPos.x + 25, cursorPos.y + 25), // Centre du bouton
                10.0f,                                      // Taille du cercle
                IM_COL32(0, 255, 0, 150)                    // Vert semi-transparent
            );
            break;
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

        drawTile(i, pairLine, cursorPos);  // Dessine la case et récupère sa position
        drawPossibleMoves(pos, cursorPos); // Affiche les déplacements possibles
    }

    ImGui::PopStyleVar();

    // Gérer la promotion des pions
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
        m_selectedPiece.reset(); // Handle right-click action
    }
}

void Board::handleClick(Position pos)
{
    // Ne pas permettre les interactions pendant la promotion
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
    Piece piece = get(pos);

    // Vérifier que la pièce appartient au joueur actif
    if (piece.type != PieceType::None && ((m_turn == PieceColor::White && piece.color == PieceColor::White) || (m_turn == PieceColor::Black && piece.color == PieceColor::Black)))
    {
        m_selectedPiece = pos;
    }
}

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

    // Vérifier si la case cible est vide (ce qui est inhabituel pour une capture)
    if (get(to).type != PieceType::None)
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

    // Réinitialiser le suivi du dernier mouvement de pion double
    bool isPawnDoubleMove = false;

    // Vérification spéciale pour le déplacement diagonal du pion
    if (piece.type == PieceType::Pawn && abs(pos.x - from.x) == 1)
    {
        // Vérifier si c'est une capture en passant
        if (isEnPassantCapture(from, pos))
        {
            // Déterminer la position du pion à capturer
            Position capturedPawnPos = {pos.x, from.y};
            // Vider la case du pion capturé
            set(capturedPawnPos, {PieceType::None, PieceColor::White});
        }
        // Capture normale: un pion ne peut se déplacer en diagonale que s'il y a une pièce ennemie à capturer
        else if (targetPiece.type == PieceType::None || targetPiece.color == piece.color)
        {
            m_selectedPiece.reset(); // Annuler la sélection
            return;
        }
    }

    // Vérifier si le déplacement est valide
    if (!piece.isMoveValid(from, pos))
    {
        m_selectedPiece.reset(); // Annuler la sélection
        return;
    }

    // Vérifier s'il s'agit d'un mouvement de deux cases pour un pion
    if (piece.type == PieceType::Pawn && abs(pos.y - from.y) == 2)
    {
        isPawnDoubleMove = true;
    }

    // Vérifier s'il y a un obstacle (uniquement pour Tour, Fou, et Reine)
    if ((piece.type == PieceType::Rook || piece.type == PieceType::Bishop || piece.type == PieceType::Queen) && !isPathClear(from, pos))
    {
        m_selectedPiece.reset();
        return;
    }

    // Vérifier si la case est occupée par une pièce adverse ou libre
    if (targetPiece.type == PieceType::None || targetPiece.color != piece.color)
    {
        // On vérifie si le roi est capturé
        if (targetPiece.type == PieceType::King)
        {
            std::cout << "King captured! Game over." << std::endl; // Debug output
            piece.hasMoved = true;                                 // Marquer la pièce comme ayant bougé
            set(pos, piece);                                       // Déplacer la pièce
            set(from, {PieceType::None, PieceColor::White});       // Vider l'ancienne case

            // Mettre à jour l'état du jeu
            m_gameOver = true;
            m_winner   = piece.color;

            // Réinitialiser la sélection et ne pas vérifier la promotion
            m_selectedPiece.reset();
            m_lastDoublePawnMove.reset();
            return;
        }

        piece.hasMoved = true;                           // Marquer la pièce comme ayant bougé
        set(pos, piece);                                 // Déplacer la pièce
        set(from, {PieceType::None, PieceColor::White}); // Vider l'ancienne case

        // Si c'était un mouvement de deux cases pour un pion, mettre à jour m_lastDoublePawnMove
        if (isPawnDoubleMove)
        {
            m_lastDoublePawnMove = pos;
        }
        else
        {
            m_lastDoublePawnMove.reset(); // Réinitialiser pour tout autre mouvement
        }

        // Vérifier si le pion doit être promu
        if (isPawnPromotion(from, pos, piece))
        {
            m_promotionInProgress = true;
            m_promotionPosition   = pos;
            m_promotionColor      = piece.color;
            // Ne pas changer de tour tant que la promotion n'est pas résolue
        }
        else
        {
            nextTurn(); // Changer le tour si pas de promotion
        }
    }

    m_selectedPiece.reset(); // Désélectionner la pièce après le déplacement
}

void Board::nextTurn()
{
    m_turn = (m_turn == PieceColor::White) ? PieceColor::Black : PieceColor::White;
}

void Board::handlePawnPromotion()
{
    if (!m_promotionInProgress)
        return;

    // Centrer la fenêtre modale
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Pawn Promotion", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Choose a piece for promotion:");

        // Les options de promotion (Dame, Tour, Fou, Cavalier)
        std::array<std::pair<PieceType, const char*>, 4> options = {
            std::make_pair(PieceType::Queen, "Queen"),
            std::make_pair(PieceType::Rook, "Rook"),
            std::make_pair(PieceType::Bishop, "Bishop"),
            std::make_pair(PieceType::Knight, "Knight")
        };

        for (auto& [type, name] : options)
        {
            if (ImGui::Button(name, ImVec2(100, 40)))
            {
                // Promouvoir le pion en la pièce choisie
                set(m_promotionPosition, {type, m_promotionColor});

                // Réinitialiser l'état de promotion
                m_promotionInProgress = false;
                ImGui::CloseCurrentPopup();

                // Passer au tour suivant maintenant que la promotion est terminée
                nextTurn();
            }
        }

        ImGui::EndPopup();
    }

    // Ouvrir la fenêtre modale si une promotion est en cours
    if (m_promotionInProgress)
    {
        ImGui::OpenPopup("Pawn Promotion");
    }
}

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
            // Ajouter la position de capture en passant
            Position enPassantPos = {pawnPos.x, from.y + direction};
            moves.push_back(enPassantPos);
        }
    }

    // Ajouter le reste des mouvements valides comme avant
    for (int x = 0; x < 8; ++x)
    {
        for (int y = 0; y < 8; ++y)
        {
            Position to = {x, y};

            // Vérification spéciale pour les pions qui se déplacent en diagonale
            if (piece.type == PieceType::Pawn && abs(to.x - from.x) == 1)
            {
                Piece targetPiece = get(to);
                // Un pion ne peut aller en diagonale que s'il y a une pièce ennemie
                if (targetPiece.type == PieceType::None && !isEnPassantCapture(from, to))
                    continue;
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