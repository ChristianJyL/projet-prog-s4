#include "app.hpp"
#include <imgui.h>
#include <iostream>
#include <string>
#include "Chess/Board.hpp"
#include "quick_imgui/quick_imgui.hpp"

constexpr ImVec4 COLOR_DARK_GREEN = ImVec4{0.0f, 0.39f, 0.0f, 1.0f};
constexpr ImVec4 COLOR_BEIGE      = ImVec4{0.96f, 0.87f, 0.70f, 1.0f};

void app::run()
{
    quick_imgui::loop(
        "Chess",
        /* init: */ [&]() {},
        /* loop: */
        [&]() {
            ImGui::ShowDemoWindow();
            ImGui::ShowAboutWindow();

            drawBoard(m_board.m_list);
        }
    );
}

void app::drawBoard(const std::vector<char>& board)
{
    bool pairLine = true;
    ImGui::Begin("Chess");
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0)); // Set item spacing to zero
    for (int i = 0; i < board.size(); ++i)
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
        std::string label(1, board.at(i));
        ImVec4      pieceColor = m_board.getColor(board.at(i));
        ImGui::PushStyleColor(ImGuiCol_Text, pieceColor);
        if (ImGui::Button(label.c_str(), ImVec2{50.f, 50.f}))
            std::cout << "Clicked button " << i << "\n";
        ImGui::PopStyleColor();
        ImGui::PopID(); // Then pop the id you pushed after you created the widget
        ImGui::PopStyleColor();
    }
    ImGui::PopStyleVar(); // Restore item spacing
    ImGui::End();
}
