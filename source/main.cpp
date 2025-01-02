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
#include "stockHandle.hpp"

sf::Sprite f[32]{};
std::string position = "e2e4";


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


std::string goNewGame(Stockfish& engine)
{
	engine.sendCommand("ucinewgame");
	engine.sendCommand("isready");

	std::string str = engine.getResponse();
	if(str.find("readyok") == std::string::npos)
		return "response error";

	return "ok";
}


std::string getNextMove(Stockfish& engine, const std::string& moves)
{
	std::string command = "position startpos moves " + moves;
	engine.sendCommand(command);
	engine.sendCommand("go");

	std::string response = engine.getResponse();
	size_t n = response.find("bestmove");
	if(n != std::string::npos)
		return response.substr(n + 9, 4);

	return "response error";
}


int main(void)
{
	try
	{
		Stockfish engine("./stockfish");
		
		engine.sendCommand("uci");
		std::string resp = engine.getResponse();
		if(resp.find("uciok") == std::string::npos)
		{
			throw std::runtime_error("Stockfish did not respond correctly to UCI command");
		}
		
		std::cout << "Stockfish initialized successfully\n";
		
		// new game started
		std::string moves = "";
		std::string response = goNewGame(engine);
		if(response == "response error")
			throw std::runtime_error("Error when starting a new game");
		
		std::cout << "New game started. Stockfish response: " << response << "\n";
		
		for (int i = 0; i < 4; ++i)
		{
			std::cout << "Enter move (e.g. e2e4): ";
			std::string move;
			std::cin >> move;
			
			moves += move + " ";
			response = getNextMove(engine, moves);
			if(response == "response error")
			{
				std::cerr << "Error: Failed to get next move from Stockfish\n";
				continue;
			}
			
			std::cout << "Stockfish suggests movement: " << response << "\n";
		}
		
		std::cout << "done.\n";
	}
	
	catch(const std::exception &e)
	{
		std::cerr << "error: " << e.what() << "\n";
		
		return -1;
	}

	return 0;
}
