/*
 * main.cpp
 * 
 * Testing few functions for my GUI Chess.
 * 
 * 01-01-2025 by madpl
 */
#include "utils.hpp"


int main(void)
{
	try
	{
		Stockfish engine("./stockfish");
		
		engine.sendCommand("uci");
		if(engine.getResponse().find("uciok") == std::string::npos)
			throw std::runtime_error("Stockfish did not respond correctly to UCI command");
		
		std::cout << "Stockfish initialized successfully\n";
		
		// new game started
		std::string moves = "";
		engine.sendCommand("ucinewgame");
		engine.sendCommand("isready");
		if(engine.getResponse().find("readyok") == std::string::npos)
			throw std::runtime_error("Stockfish is not ready to play");
		
		for(int i = 0; i < 4; ++i)
		{
			std::string playerMove;
			std::cout << "Enter move (e.g. e2e4): ";
			std::cin >> playerMove;
			
			// adding player movement to history
			moves += (moves.empty() ? "" : " ") + playerMove;
			
			// Stockfish move download
			std::string stockfishMove = getNextMove(engine, moves);
			if(stockfishMove == "response error")
			{
				std::cerr << "Error: Failed to get move from Stockfish\n";
				
				break;
			}
			
			std::cout << "Stockfish suggests movement: " << stockfishMove << "\n";
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
