/*
 * main_stock.cpp
 *
 * Simple graphics user interface chess code that stockfish uses.
 * I wrote this code as an exercise in C++ programming:))
 * 
 * 06-12-2024 by madpl
 */
#include <iostream>
#include "stockHandle.hpp"
#include "defines.hpp"
#include "chess.hpp"


int main(void)
{
	try
	{
		// logo
		std::cout << "guichess by madpl 2024 started.\n";
		
		sf::RenderWindow window(sf::VideoMode(BOARD_SIZE, BOARD_SIZE), "guichess by madpl - 2024");
		window.setPosition(sf::Vector2i(600, 200));
		window.setFramerateLimit(60);
		
		Chess chess;
		chess.setupBoard();
		
		Stockfish engine("./stockfish");
		engine.sendCommand("uci");
		if(engine.getResponse().find("uciok") == std::string::npos)
		{
			engine.sendCommand("quit");
			std::cerr << "engine: not uciok!\n";
			
			return -1;
		}
		
		// setting some additional engine options
		engine.sendCommand("setoption name Skill Level value 5");
		engine.sendCommand("setoption name Minimum Thinking Time value 3000");
		engine.sendCommand("uci");
		std::cout << engine.getResponse() << "\n";
		
		engine.sendCommand("isready");
		if(engine.getResponse().find("readyok") != std::string::npos)
			std::cout << "engine: readyok\n";
		else
		{
			engine.sendCommand("quit");
			std::cerr << "engine: not readyok!\n";
			
			return -1;
		}
		
		while(window.isOpen())
		{
			sf::Event event;
			while(window.pollEvent(event))
			{
				if(event.type == sf::Event::Closed)
				{
					engine.sendCommand("quit");
					window.close();
				}
				
				chess.handleMouseEvent(event, window);
			}
			
			window.clear();
			chess.draw(window);
			window.display();
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