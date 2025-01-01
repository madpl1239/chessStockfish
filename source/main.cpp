/*
 * main.cpp
 * 
 */
#include <SFML/Graphics.hpp>
#include <iostream>


std::string toChessNote(sf::Vector2f piece)
{
	std::string str;
	
	str += char((piece.x / TILE_SIZE) + 97);
	str += char(7 - (piece.y / TILE_SIZE) + 49);
	
	return str;
}


sf::Vector2f toCoords(char a, char b)
{
	int x = int(a) - 97;
	int y = 7 - int(b) + 49;
	
	return sf::Vector2f(x * TILE_SIZE, y * TILE_SIZE);
}


int main(void)
{
	
	return 0;
}
