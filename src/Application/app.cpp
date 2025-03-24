#include "app.hpp"
#include <imgui.h>
#include <iostream>
#include <string>
#include "Chess/Board.hpp"
#include "quick_imgui/quick_imgui.hpp"

void app::run()
{
    quick_imgui::loop(
        "Chess",
        {
            .init                     = [&]() { m_board = Board(); },
            .loop                     = [&]() {
                m_board.drawBoard();

                // Afficher un message si la partie est terminée
            if (m_board.isGameOver())
            {
                std::cout << "Game is over, opening popup" << std::endl; // Debug output
                ImGui::OpenPopup("Game Over");
            }
            
            //Popup modal
            if (ImGui::BeginPopupModal("Game Over", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                PieceColor winner = m_board.getWinner();
                const char* winnerText = (winner == PieceColor::White) ? "White" : "Black";
                ImGui::Text("%s wins! The king has been captured.", winnerText);
                
                if (ImGui::Button("New Game", ImVec2(120, 0)))
                {
                    m_board = Board(); 
                    ImGui::CloseCurrentPopup();
                    std::cout << "Starting new game" << std::endl; 
                }
                
                ImGui::SameLine();
                
                if (ImGui::Button("Close", ImVec2(120, 0)))
                {
                    ImGui::CloseCurrentPopup();
                }
                
                ImGui::EndPopup();
            } },
            .key_callback             = [](int key, int scancode, int action, int mods) { std::cout << "Key: " << key << " Scancode: " << scancode << " Action: " << action << " Mods: " << mods << '\n'; },
            .mouse_button_callback    = [](int button, int action, int mods) { std::cout << "Button: " << button << " Action: " << action << " Mods: " << mods << '\n'; },
            .cursor_position_callback = [](double xpos, double ypos) { std::cout << "Position: " << xpos << ' ' << ypos << '\n'; },
            .scroll_callback          = [](double xoffset, double yoffset) { std::cout << "Scroll: " << xoffset << ' ' << yoffset << '\n'; },
            .window_size_callback     = [](int width, int height) { std::cout << "Resized: " << width << ' ' << height << '\n'; },
        }
    );
}