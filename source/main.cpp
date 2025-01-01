/*
 * main.cpp
 * 
 * Testing few functions.
 * 
 * 01-01-2025 by madpl
 */
#include <SFML/Graphics.hpp>
#include <iostream>
#include "defines.hpp"

sf::Sprite f[32]{};
std::string position = "a2a4";

std::string toChess(sf::Vector2f piece)
{
	std::string str;
	
	str += static_cast<char>(97 + (piece.x / TILE_SIZE));
	str += static_cast<char>(7 - (piece.y / TILE_SIZE) + 49);
	
	return str;
}


sf::Vector2f toCoords(char a, char b)
{
	int x = static_cast<int>(a) - 97;
	int y = 7 - static_cast<int>(b) + 49;
	
	return sf::Vector2f(x * TILE_SIZE, y * TILE_SIZE);
}


void move(std::string str)
{
	sf::Vector2f oldPos = toCoords(str[0], str[1]);
	sf::Vector2f newPos = toCoords(str[2], str[3]);
	
	for(int i = 0; i < 32; ++i)
		if(f[i].getPosition() == newPos)
			f[i].setPosition(-100, -100);
		
	for(int i = 0; i < 32; ++i)
		if(f[i].getPosition() == oldPos)
			f[i].setPosition(newPos);
	
	// castling if the king not moved yet
	if(str == "e1g1") // king's move
		if(position.find("e1") == -1) 
			move("h1f1"); // rook's move
		
	if(str == "e8g8")
		if(position.find("e8") == -1)
			move("h8f8");
		
	if(str == "e1c1")
		if(position.find("e1") == -1) 
			move("a1d1");
		
	if(str == "e8c8") 
		if(position.find("e8") == -1)
			move("a8d8");
}


int main(void)
{
	sf::Vector2f vec{};
	std::string str{};
	
	std::cout << "testing toCoords()\n";
	std::cout << "enter two chars: ";
	std::cin >> str;
	
	vec = toCoords(str[0], str[1]);
	
	std::cout << "sf_vector(" << vec.x << ", " << vec.y << ") --> "
			  << "board_coords(" << vec.x / TILE_SIZE << ", " << vec.y / TILE_SIZE
			  << ")\n";
	
	return 0;
}
