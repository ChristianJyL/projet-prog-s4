#include "Board.hpp"
#include <imgui.h>
#include <array>
#include <iostream>
#include <string>
#include <vector>

constexpr ImVec4 COLOR_DARK_GREEN = ImVec4{0.0f, 0.39f, 0.0f, 1.0f}; // utiliser enum ?
constexpr ImVec4 COLOR_BEIGE      = ImVec4{0.96f, 0.87f, 0.70f, 1.0f};


Board::Board() {
    initializeBoard();
}

void Board::initializeBoard() {
    // Initialisation des pièces blanches
std::array<PieceType, 8> Pieces = {PieceType::Rook, PieceType::Knight, PieceType::Bishop, PieceType::Queen, PieceType::King, PieceType::Bishop, PieceType::Knight, PieceType::Rook};
for (int i = 0; i < 8; ++i) {
    m_list[i] = {Pieces[i], PieceColor::White};
    m_list[i + 8] = {PieceType::Pawn, PieceColor::White};
}
// Initialisation des pièces noires
for (int i = 0; i < 8; ++i) {
    m_list[56 + i] = {Pieces[i], PieceColor::Black};
    m_list[48 + i] = {PieceType::Pawn, PieceColor::Black};
}
}

Piece Board::get(Position pos) const {
    return m_list.at(pos.x + pos.y * 8);
}

void Board::set(Position pos, Piece piece) {
    m_list.at(pos.x + pos.y * 8) = piece;
}

void Board::move(Position from, Position to) {
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
    Piece piece = m_list.at(index);
    std::string label = std::string(1, piece.toChar());
    ImVec4 pieceColor = getPieceColor(piece);

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
    if (!m_selectedPiece) return;

    Piece selectedPiece = get(*m_selectedPiece);
    if (selectedPiece.type == PieceType::None) return;

    // Vérifier si la case est un déplacement valide
    for (const Position& move : getValidMoves(*m_selectedPiece)) {
        if (move.x == pos.x && move.y == pos.y) {
            ImGui::GetWindowDrawList()->AddCircleFilled(
                ImVec2(cursorPos.x + 25, cursorPos.y + 25), // Centre du bouton
                10.0f, // Taille du cercle
                IM_COL32(0, 255, 0, 150) // Vert semi-transparent
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
        ImVec2 cursorPos;

        drawTile(i, pairLine, cursorPos); // Dessine la case et récupère sa position
        drawPossibleMoves(pos, cursorPos); // Affiche les déplacements possibles
    }

    ImGui::PopStyleVar();
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

bool Board::isPathClear(Position from, Position to) const {
    int dx = to.x - from.x;
    int dy = to.y - from.y;

    int stepX = (dx == 0) ? 0 : (dx > 0 ? 1 : -1);
    int stepY = (dy == 0) ? 0 : (dy > 0 ? 1 : -1);

    Position checkPos = from;
    checkPos.x += stepX;
    checkPos.y += stepY;

    while (checkPos.x != to.x || checkPos.y != to.y) {
        if (!checkPos.isValid() || get(checkPos).type != PieceType::None) {
            return false; // Une pièce bloque le passage
        }
        checkPos.x += stepX;
        checkPos.y += stepY;
    }

    return true;
}

void Board::movePiece(Position pos)
{
    if (!m_selectedPiece) return;

    Position from = *m_selectedPiece;
    Piece piece = get(from);
    Piece targetPiece = get(pos);

    // Vérifier si le déplacement est valide
    if (!piece.isMoveValid(from, pos)) {
        m_selectedPiece.reset(); // Annuler la sélection
        return;
    }

    // Vérifier s'il y a un obstacle (uniquement pour Tour, Fou, et Reine)
    if ((piece.type == PieceType::Rook || piece.type == PieceType::Bishop || piece.type == PieceType::Queen) &&
        !isPathClear(from, pos)) {
        m_selectedPiece.reset();
        return;
        }

    // Vérifier si la case est occupée par une pièce adverse ou libre
    if (targetPiece.type == PieceType::None || targetPiece.color != piece.color) {
        piece.hasMoved = true; // Marquer la pièce comme ayant bougé
        set(pos, piece); // Déplacer la pièce
        set(from, {PieceType::None, PieceColor::White}); // Vider l'ancienne case
        nextTurn(); // Changer le tour
    }

    m_selectedPiece.reset(); // Désélectionner la pièce après le déplacement
}

void Board::nextTurn()
{
    m_turn = (m_turn == PieceColor::White) ? PieceColor::Black : PieceColor::White;
}

std::vector<Position> Board::getValidMoves(Position from) const {
    std::vector<Position> moves;
    Piece piece = get(from);
    if (piece.type == PieceType::None) return moves;

    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            Position to = {x, y};
            if (!piece.isMoveValid(from, to)) continue;
            if ((piece.type == PieceType::Rook || piece.type == PieceType::Bishop || piece.type == PieceType::Queen) && !isPathClear(from, to)) continue;
            Piece targetPiece = get(to);
            if (targetPiece.type == PieceType::None || targetPiece.color != piece.color) moves.push_back(to);
        }
    }
    return moves;
}

