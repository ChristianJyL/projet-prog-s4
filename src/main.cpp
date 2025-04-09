#include <glad/glad.h> // Assurez-vous d'inclure glad avant GLFW
#include "Application/app.hpp"
#include <iostream> // Include for std::cout
#include "quick_imgui/quick_imgui.hpp"

int main()
{
    app gameApp; // Define gameApp outside the lambda

    quick_imgui::loop( // Ensure quick_imgui is a valid namespace or class
        "Chess",
        {
            .init                     = [&]() { gameApp.init(); }, // Appel correct de la méthode init()
            .loop                     = [&]() { 
                gameApp.update();
            },
            .key_callback             = [](int key, int scancode, int action, int mods) { std::cout << "Key: " << key << " Scancode: " << scancode << " Action: " << action << " Mods: " << mods << '\n'; },
            .mouse_button_callback    = [](int button, int action, int mods) { std::cout << "Button: " << button << " Action: " << action << " Mods: " << mods << '\n'; },
            .cursor_position_callback = [](double xpos, double ypos) {},
            .scroll_callback          = [](double xoffset, double yoffset) { std::cout << "Scroll: " << xoffset << ' ' << yoffset << '\n'; },
            .window_size_callback     = [](int width, int height) { std::cout << "Resized: " << width << ' ' << height << '\n'; },
        }
    );
}