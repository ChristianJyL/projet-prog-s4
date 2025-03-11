#pragma once
#include <cmath>

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
    bool hasMoved = false;


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

    bool isMoveValid(Position from, Position to) const {
        int dx = to.x - from.x;
        int dy = to.y - from.y;

        switch (type) {
        case PieceType::Pawn: {
            int direction = (color == PieceColor::White) ? 1 : -1; // Sens de déplacement

            // Avance d'une case
            if (dx == 0 && dy == direction) {
                return true;
            }

            // Avance de deux cases si c'est le premier mouvement
            if (!hasMoved && dx == 0 && dy == 2 * direction) {
                return true;
            }

            return false;
        }

        case PieceType::Rook:
            return (dx == 0 || dy == 0); // Déplacement en ligne droite

        case PieceType::Knight:
            return ( (abs(dx) == 2 && abs(dy) == 1) || (abs(dx) == 1 && abs(dy) == 2) ); // Déplacement en "L"

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





};