#include "app.hpp"
#include <imgui.h>
#include <string>

void app::run()
{
    quick_imgui::loop(
        "Chess",
        /* init: */ [&]() {},
        /* loop: */
        [&]() {
            ImGui::ShowDemoWindow();
            ImGui::ShowAboutWindow();
            drawBoard();
        }
    );
}

void app::drawBoard()
{
    bool pairLine = true;
    ImGui::Begin("Chess");
    for (int i = 0; i < 64; ++i)
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
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{1.f, 0.f, 0.f, 1.f}); // Changes the color of all buttons until we call ImGui::PopStyleColor(). There is also ImGuiCol_ButtonActive and ImGuiCol_ButtonHovered
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{1.f, 1.f, 0.f, 1.f}); // Changes the color of all buttons until we call ImGui::PopStyleColor(). There is also ImGuiCol_ButtonActive and ImGuiCol_ButtonHovered
            }
        }
        else
        {
            if (i % 2 == 0)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{1.f, 1.f, 0.f, 1.f}); // Changes the color of all buttons until we call ImGui::PopStyleColor(). There is also ImGuiCol_ButtonActive and ImGuiCol_ButtonHovered
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{1.f, 0.f, 0.f, 1.f}); // Changes the color of all buttons until we call ImGui::PopStyleColor(). There is also ImGuiCol_ButtonActive and ImGuiCol_ButtonHovered
            }
        }

        ImGui::PushID(i); // When some ImGui items have the same label (for exemple the next two buttons are labeled "Yo") ImGui needs you to specify an ID so that it can distinguish them. It can be an int, a pointer, a string, etc.
                          // You will definitely run into this when you create a button for each of your chess pieces, so remember to give them an ID!
        if (ImGui::Button(std::to_string(i).c_str(), ImVec2{50.f, 50.f}))
            std::cout << "Clicked button " << i << "\n";
        ImGui::PopID(); // Then pop the id you pushed after you created the widget
        ImGui::PopStyleColor();
    }
    ImGui::End();
}
