#pragma once

enum class PieceType {
    None,
    Pawn,
    Rook,
    Knight,
    Bishop,
    Queen,
    King
};

enum class PieceColor {
    White,
    Black
};

struct Piece {
    PieceType type;
    PieceColor color;

    bool isEmpty() const { return type == PieceType::None; }

    char toChar() const {
        switch (type) {
        case PieceType::Pawn:   return 'P';
        case PieceType::Rook:   return 'R';
        case PieceType::Knight: return 'N';
        case PieceType::Bishop: return 'B';
        case PieceType::Queen:  return 'Q';
        case PieceType::King:   return 'K';
        default: return ' '; // Case vide
        }
    }

    bool isMoveValid(Position from, Position to, bool isFirstMove) const {
        int dx = to.x - from.x;
        int dy = to.y - from.y;

        switch (type) {
        case PieceType::Pawn: {
            int direction = (color == PieceColor::White) ? 1 : -1; // Sens de déplacement

            // Avance d'une case
            if (dx == 0 && dy == direction) {
                return true;
            }
            return false;
        }

        case PieceType::Rook:
            return (dx == 0 || dy == 0); // Déplacement en ligne droite

        case PieceType::Knight:
            return (dx * dy == 2); // Mouvement en L

        case PieceType::Bishop:
            return (abs(dx) == abs(dy)); // Déplacement en diagonale

        case PieceType::Queen:
            return (dx == 0 || dy == 0 || abs(dx) == abs(dy)); // Tour + Fou combinés

        case PieceType::King:
            return (abs(dx) <= 1 && abs(dy) <= 1); // Une case dans n'importe quelle direction

        default:
            return false;
        }
    }

    std::vector<Position> getValidMoves(Position from) const {
        std::vector<Position> moves;

        for (int dx = -7; dx <= 7; ++dx) {
            for (int dy = -7; dy <= 7; ++dy) {
                Position to = {from.x + dx, from.y + dy};

                if (to.isValid() && isMoveValid(from, to, false)) {
                    moves.push_back(to);
                }
            }
        }

        return moves;
    }


};