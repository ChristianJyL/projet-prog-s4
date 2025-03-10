#pragma once

struct Position {
    int x;
    int y;

    bool isValid() const {
        return x >= 0 && x < 8 && y >= 0 && y < 8;
    }
};