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
        /* init: */ [&]() {},
        /* loop: */
        [&]() {
            m_board.drawBoard();
        }
    );
}
