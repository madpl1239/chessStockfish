/*
 * utils.cpp
 * 
 * Definition of several utility functions.
 * 
 * 03-01-2025 by madpl
 */
#include "utils.hpp"


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


void move(sf::Sprite f[], std::string str, std::string& globalPos)
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
		if(globalPos.find("e1") == -1) 
			move(f, "h1f1", globalPos); // rook's move

	if(str == "e8g8")
		if(globalPos.find("e8") == -1)
			move(f, "h8f8", globalPos);

	if(str == "e1c1")
		if(globalPos.find("e1") == -1) 
			move(f, "a1d1", globalPos);

	if(str == "e8c8") 
		if(globalPos.find("e8") == -1)
			move(f, "a8d8", globalPos);
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
	// sending full movement history
	std::string command = "position startpos moves " + moves;
	std::cout << "[DEBUG] Full move history: " << command << "\n";
	engine.sendCommand(command);

	engine.sendCommand("go depth 3");
	std::string response = engine.getResponse();

	// extracting the 'bestmove' movement
	size_t bestmoveIdx = response.find("bestmove");
	if(bestmoveIdx != std::string::npos)
	{
		size_t endIdx = response.find(' ', bestmoveIdx + 9);
		std::string stockfishMove = (endIdx != std::string::npos) 
								? response.substr(bestmoveIdx + 9, endIdx - (bestmoveIdx + 9))
								: response.substr(bestmoveIdx + 9);
		
		// adding Stockfish Movement to History
		moves += " " + stockfishMove;
		
		return stockfishMove;
	}

	return "response error";
}
