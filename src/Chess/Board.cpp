#include "Board.hpp"
#include <imgui.h>
#include <string>
#include <vector>

constexpr ImVec4 COLOR_DARK_GREEN = ImVec4{0.0f, 0.39f, 0.0f, 1.0f}; // utiliser enum ?
constexpr ImVec4 COLOR_BEIGE      = ImVec4{0.96f, 0.87f, 0.70f, 1.0f};

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

void Board::drawBoard()
{
    bool pairLine = true;
    ImGui::Begin("Chess");
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0)); // Set item spacing to zero
    for (int i = 0; i < m_list.size(); ++i)
    {
        if (i % 8 != 0)
        {
            ImGui::SameLine(); // Draw the next ImGui widget on the same line as the previous one. Otherwise it would be below it
        }
        else
        {
            pairLine = !pairLine;
        }

        if (pairLine)
        {
            if (i % 2 == 0)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, COLOR_DARK_GREEN); // Changes the color of all buttons until we call ImGui::PopStyleColor(). There is also ImGuiCol_ButtonActive and ImGuiCol_ButtonHovered
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Button, COLOR_BEIGE); // Changes the color of all buttons until we call ImGui::PopStyleColor(). There is also ImGuiCol_ButtonActive and ImGuiCol_ButtonHovered
            }
        }
        else
        {
            if (i % 2 == 0)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, COLOR_BEIGE); // Changes the color of all buttons until we call ImGui::PopStyleColor(). There is also ImGuiCol_ButtonActive and ImGuiCol_ButtonHovered
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Button, COLOR_DARK_GREEN); // Changes the color of all buttons until we call ImGui::PopStyleColor(). There is also ImGuiCol_ButtonActive and ImGuiCol_ButtonHovered
            }
        }

        ImGui::PushID(i); // When some ImGui items have the same label (for exemple the next two buttons are labeled "Yo") ImGui needs you to specify an ID so that it can distinguish them. It can be an int, a pointer, a string, etc.
                          // You will definitely run into this when you create a button for each of your chess pieces, so remember to give them an ID!
        std::string label(1, m_list.at(i));
        ImVec4      pieceColor = getColor(m_list.at(i));
        ImGui::PushStyleColor(ImGuiCol_Text, pieceColor);
        ImGui::Button(label.c_str(), ImVec2{50.f, 50.f});
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            handleClick(i % 8, i / 8);
        }
        else if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        {
            // Reset the board or handle right-click action
            m_selectedPiece.reset();
        }

        ImGui::PopStyleColor();
        ImGui::PopID(); // Then pop the id you pushed after you created the widget
        ImGui::PopStyleColor();
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