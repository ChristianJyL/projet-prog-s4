#include "GameMode.hpp"
#include <array>

//On implémente des méthodes pour un chess classique

void GameMode::initializeBoard(std::vector<Piece>& board) {
    // Initialisation des pièces blanches
    std::array<PieceType, 8> pieces = {PieceType::Rook, PieceType::Knight, PieceType::Bishop, PieceType::Queen, PieceType::King, PieceType::Bishop, PieceType::Knight, PieceType::Rook};
    for (int i = 0; i < 8; ++i) {
        board[i] = {pieces[i], PieceColor::White};
        board[i + 8] = {PieceType::Pawn, PieceColor::White};
    }
    
    // Initialisation des pièces noires
    for (int i = 0; i < 8; ++i) {
        board[56 + i] = {pieces[i], PieceColor::Black};
        board[48 + i] = {PieceType::Pawn, PieceColor::Black};
    }
    
    // Initialiser les cases vides
    for (int y = 2; y < 6; ++y) {
        for (int x = 0; x < 8; ++x) {
            board[x + y * 8] = {PieceType::None, PieceColor::White};
        }
    }
}

//Méthode côté logique
bool GameMode::isValidMove(const std::vector<Piece>& board, Position from, Position to, const Piece& piece) {
    if (!piece.isMoveValid(from, to)) {
        return false;
    }
    
    // Vérification spéciale pour les pions
    if (piece.type == PieceType::Pawn) {
        int dx = to.x - from.x;
        int dy = to.y - from.y;
        int direction = (piece.color == PieceColor::White) ? 1 : -1;
        
        // Vérification pour le double mouvement de pion
        if (dx == 0 && abs(dy) == 2) {
            // Vérifier s'il y a une pièce sur le chemin
            Position middlePos = {from.x, from.y + direction};
            if (board[middlePos.x + middlePos.y * 8].type != PieceType::None) {
                return false; // Une pièce bloque le chemin
            }
        }
        
        // Si c'est un mouvement diagonal (capture)
        if (abs(dx) == 1 && dy == direction) {
            // Pour capturer, il DOIT y avoir une pièce ennemie à la position cible
            // OU c'est une capture en passant (vérifiée par la Board)
            Piece targetPiece = board[to.x + to.y * 8];
            
            if (targetPiece.type == PieceType::None) {
                // La case est vide, ça pourrait être une capture en passant
                // Vérifier si c'est une position valide pour l'en passant
                int expectedY = (piece.color == PieceColor::White) ? 4 : 3;
                
                if (from.y != expectedY) {
                    return false; // Ce n'est pas une position valide pour l'en passant
                }
                
                // Vérifier si un pion ennemi est adjacent (potentiellement capturé en passant)
                Position adjacentPos = {to.x, from.y};
                Piece adjacentPiece = board[adjacentPos.x + adjacentPos.y * 8];
                
                if (adjacentPiece.type != PieceType::Pawn || adjacentPiece.color == piece.color) {
                    return false; // Pas de pion ennemi adjacent, donc pas d'en passant possible
                }
                
                // Note: La Board validera complètement si ce pion a effectivement fait un double mouvement
            } else if (targetPiece.color == piece.color) {
                return false; // On ne peut pas capturer une pièce de sa couleur
            }
        }
    }
    
    // Vérifier s'il y a un obstacle sur le chemin (pour Tour, Fou, et Reine)
    if ((piece.type == PieceType::Rook || piece.type == PieceType::Bishop || piece.type == PieceType::Queen) && !isPathClear(board, from, to)) {
        return false;
    }
    
    // Vérifier si la case d'arrivée est libre ou contient une pièce adverse
    Piece targetPiece = board[to.x + to.y * 8];
    if (targetPiece.type != PieceType::None && targetPiece.color == piece.color) {
        return false;
    }
    
    return true;
}

void GameMode::executeMove(std::vector<Piece>& board, Position from, Position to) {
    board[to.x + to.y * 8] = board[from.x + from.y * 8];
    board[from.x + from.y * 8] = {PieceType::None, PieceColor::White};
    
    board[to.x + to.y * 8].hasMoved = true;
}

ImVec4 GameMode::getTileColor(bool isPairLine, int index, Position pos) const {
    constexpr ImVec4 COLOR_DARK_GREEN = ImVec4{0.0f, 0.39f, 0.0f, 1.0f};
    constexpr ImVec4 COLOR_BEIGE = ImVec4{0.96f, 0.87f, 0.70f, 1.0f};
    return ((isPairLine && index % 2 == 0) || (!isPairLine && index % 2 != 0)) ? COLOR_DARK_GREEN : COLOR_BEIGE;
}

//Méthode côté logique
bool GameMode::isPathClear(const std::vector<Piece>& board, Position from, Position to) const {
    int dx = to.x - from.x;
    int dy = to.y - from.y;

    int stepX = (dx == 0) ? 0 : (dx > 0 ? 1 : -1);
    int stepY = (dy == 0) ? 0 : (dy > 0 ? 1 : -1);

    Position checkPos = from;
    checkPos.x += stepX;
    checkPos.y += stepY;

    while (checkPos.x != to.x || checkPos.y != to.y) {
        if (!checkPos.isValid() || board[checkPos.x + checkPos.y * 8].type != PieceType::None) {
            return false; // Une pièce bloque le passage
        }
        checkPos.x += stepX;
        checkPos.y += stepY;
    }

    return true;
}

