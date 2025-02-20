#include "Board.hpp"
#include <imgui.h>
#include <string>
#include <vector>

constexpr ImVec4 COLOR_DARK_GREEN = ImVec4{0.0f, 0.39f, 0.0f, 1.0f}; // utiliser enum ?
constexpr ImVec4 COLOR_BEIGE      = ImVec4{0.96f, 0.87f, 0.70f, 1.0f};

// Color of the pieces
ImVec4 Board::getColor(char piece)
{
    if (piece >= 'a' && piece <= 'z')
    {
        return {0.0f, 0.0f, 0.0f, 1.0f}; // black
    }
    else
    {
        return {1.0f, 1.0f, 1.0f, 1.0f}; // white
    }
}

ImVec4 Board::getTileColor(bool isPairLine, int index) const
{
    return ((isPairLine && index % 2 == 0) || (!isPairLine && index % 2 != 0)) ? COLOR_DARK_GREEN : COLOR_BEIGE;
}

void Board::drawTile(int index, bool pairLine)
{
    // Determine if the tile should be highlighted
    bool highlight = false;
    if (m_selectedPiece)
    {
        char targetPiece = m_list.at(index);
        if (targetPiece == 0 || (m_whiteTurn && targetPiece >= 'a' && targetPiece <= 'z') || (!m_whiteTurn && targetPiece >= 'A' && targetPiece <= 'Z'))
        {
            highlight = true;
        }
    }

    // Push the appropriate button color
    ImVec4 tileColor = getTileColor(pairLine, index);
    if (highlight)
    {
        tileColor = ImVec4{0.0f, 1.0f, 0.0f, 1.0f}; // Highlight color
    }
    ImGui::PushStyleColor(ImGuiCol_Button, tileColor);

    // Set piece-related colors and labels
    std::string label(1, m_list.at(index));
    ImVec4      pieceColor = getColor(m_list.at(index));

    ImGui::PushID(index); // Assign unique ID for each square
    ImGui::PushStyleColor(ImGuiCol_Text, pieceColor);

    // Draw the button representing the square
    ImGui::Button(label.c_str(), ImVec2{50.f, 50.f});

    // Handle mouse interactions
    handleMouseInteraction(index);

    ImGui::PopStyleColor(); // Pop piece color
    ImGui::PopID();         // Pop unique ID
    ImGui::PopStyleColor(); // Pop tile color
}

void Board::drawBoard()
{
    ImGui::Begin("Chess");
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0)); // Set item spacing to zero

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

        drawTile(i, pairLine);
    }

    ImGui::PopStyleVar(); // Restore item spacing
    ImGui::End();
}

char Board::get(int x, int y)
{
    return m_list.at(x + y * 8);
}
void Board::set(int x, int y, char piece)
{
    m_list.at(x + y * 8) = piece;
}

void Board::move(Position from, Position to)
{
    char piece = get(from.x, from.y);
    set(to.x, to.y, piece);
    set(from.x, from.y, 0);
}

void Board::handleMouseInteraction(int index)
{
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        handleClick(index % 8, index / 8);
    }
    else if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
    {
        m_selectedPiece.reset(); // Handle right-click action
    }
}

void Board::handleClick(int x, int y)
{
    char piece = get(x, y);

    // Si aucune pièce n'est sélectionnée, on essaye de sélectionner
    if (!m_selectedPiece)
    {
        selectPiece(x, y);
    }
    // Si une pièce est déjà sélectionnée
    else
    {
        Position from = *m_selectedPiece;
        // Si on clique sur une pièce du même joueur -> on sélectionne celle-ci à la place
        if (piece != 0 && ((m_whiteTurn && piece >= 'A' && piece <= 'Z') || (!m_whiteTurn && piece >= 'a' && piece <= 'z')))
        {
            selectPiece(x, y);
        }
        // Sinon, on tente un déplacement (vide ou capture)
        else
        {
            movePiece(x, y);
        }
    }
}

void Board::selectPiece(int x, int y)
{
    char piece = get(x, y);

    // Vérifier que la pièce appartient au joueur actif
    if (piece != 0 && ((m_whiteTurn && piece >= 'A' && piece <= 'Z') || (!m_whiteTurn && piece >= 'a' && piece <= 'z')))
    {
        m_selectedPiece = Position{.x = x, .y = y};
    }
}

void Board::movePiece(int x, int y)
{
    if (!m_selectedPiece)
        return;

    char targetPiece = get(x, y);

    // Vérifier si on essaie de capturer une pièce ennemie ou d'aller sur une case vide
    if (targetPiece == 0 || (m_whiteTurn && targetPiece >= 'a' && targetPiece <= 'z') || (!m_whiteTurn && targetPiece >= 'A' && targetPiece <= 'Z'))
    {
        move(*m_selectedPiece, Position{.x = x, .y = y});
        nextTurn();
    }

    // Désélectionner la pièce après le coup
    m_selectedPiece.reset();
}

void Board::nextTurn()
{
    m_whiteTurn = !m_whiteTurn;
}
