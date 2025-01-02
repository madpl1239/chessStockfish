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


std::string getNextMove(Stockfish& engine, std::string& moves)
{
	// Przesyłanie pełnej historii ruchów
	std::string command = "position startpos moves " + moves;
	engine.sendCommand(command);

	// Wykonanie analizy
	engine.sendCommand("go depth 3");
	std::string response = engine.getResponse();

	// Wyodrębnienie ruchu 'bestmove'
	size_t bestmoveIdx = response.find("bestmove");
	if(bestmoveIdx != std::string::npos)
	{
		size_t endIdx = response.find(' ', bestmoveIdx + 9);
		std::string stockfishMove = (endIdx != std::string::npos) 
								? response.substr(bestmoveIdx + 9, endIdx - (bestmoveIdx + 9))
								: response.substr(bestmoveIdx + 9);
		
		// Dodanie ruchu Stockfisha do historii
		moves += " " + stockfishMove;
		
		return stockfishMove;
	}

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
		engine.sendCommand("ucinewgame");
		engine.sendCommand("isready");
		if(engine.getResponse().find("readyok") == std::string::npos)
			throw std::runtime_error("Stockfish nie jest gotowy do gry");
		
		for(int i = 0; i < 4; ++i)
		{
			std::string playerMove;
			std::cout << "Enter move (e.g. e2e4): ";
			std::cin >> playerMove;
			
			// Dodanie ruchu gracza do historii
			moves += (moves.empty() ? "" : " ") + playerMove;
			
			// Pobranie ruchu Stockfisha
			std::string stockfishMove = getNextMove(engine, moves);
			if(stockfishMove == "response error")
			{
				std::cerr << "Error: Failed to get move from Stockfish\n";
				break;
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
