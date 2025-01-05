/*
 * utils.cpp
 * 
 * Definition of several utility functions.
 * 
 * 03-01-2025 by madpl
 */
#include "utils.hpp"


std::string goNewGame(Stockfish& engine)
{
	engine.sendCommand("ucinewgame");
	engine.sendCommand("isready");

	std::string str = engine.getResponse();
	if(str.find("readyok") == std::string::npos)
		return "response error";

	#ifdef DEBUG
	std::cout << "[DEBUG] ucinewgame\n";
	std::cout << "[DEBUG] readyok\n";
	#endif	
	
	return "ok";
}


std::string getNextMove(Stockfish& engine, std::string& moves)
{
	// sending full movement history
	std::string command = "position startpos moves" + moves;
	engine.sendCommand(command);

	engine.sendCommand("go depth 2");
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
