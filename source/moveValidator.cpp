/*
 * moveValidator.cpp
 * 
 * 14-01-2025 by madpl
 */
#include <cctype>
#include "moveValidator.hpp"


MoveValidator::MoveValidator(const std::vector<std::vector<char>>& board):
	board(board)
{
	// empty
}


bool MoveValidator::isMoveValid(const std::string& move) const
{
    if(move.length() != 4)
        return false;
    
    char col1 = move[0];
    char row1 = move[1];
    char col2 = move[2];
    char row2 = move[3];
    int x1 = col1 - 'a';
    int y1 = row1 - '1';
    int x2 = col2 - 'a';
    int y2 = row2 - '1';
    
    if(!isWithinBounds(x1, y1) or !isWithinBounds(x2, y2))
        return false;
    
    char piece = board[y1][x1];
    if(piece == ' ')
        return false;
    
    return isLegalMoveForPiece(piece, move) and isPathClear(move);
}


bool MoveValidator::isWithinBounds(int x, int y) const
{
    return x >= 0 and x < 8 and y >= 0 and y < 8;
}


bool MoveValidator::isPathClear(const std::string& move) const
{
    return true; // Placeholder for path checking logic
}


bool MoveValidator::isLegalMoveForPiece(char piece, const std::string& move) const
{
    return true; // Placeholder for piece-specific move logic
}
