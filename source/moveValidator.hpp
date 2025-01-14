/*
 * moveValidator.hpp
 * 
 * 14-01-2025 by madpl
 */
#pragma once

#include <string>
#include <vector>


class MoveValidator
{
public:
	MoveValidator(const std::vector<std::vector<char>>& board);
	bool isMoveValid(const std::string& move) const;

private:
	bool isWithinBounds(int x, int y) const;
	bool isPathClear(const std::string& move) const;
	bool isLegalMoveForPiece(char piece, const std::string& move) const;
	
	const std::vector<std::vector<char>>& board;
};
