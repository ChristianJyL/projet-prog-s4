#pragma once

struct Position {
    int x;
    int y;

    bool isValid() const {
        return x >= 0 && x < 8 && y >= 0 && y < 8;
    }

    //pour utiliser une map
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
    bool operator<(const Position& other) const {
        if (x != other.x)
            return x < other.x;
        return y < other.y;
    }
};