/*
 * main_stock.cpp
 *
 * Testing sockHandle class.
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
		sf::RenderWindow window(sf::VideoMode(BOARD_SIZE, BOARD_SIZE), "Chess by madpl - 2024");
		window.setPosition(sf::Vector2i(500, 200));
		window.setFramerateLimit(60);
		
		Chess chess;
		chess.setupBoard();
		
		Stockfish engine("./stockfish");
		
		engine.sendCommand("uci");
		std::string resp = engine.getResponse();
		
		std::cout << "resp.size() = " << resp.size() << "\n";
		std::cout << "resp = " << resp << "\n";
		
		resp.clear();
		engine.sendCommand("isready");
		resp = engine.getResponse();
		
		std::cout << "resp.size() = " << resp.size() << "\n";
		std::cout << "resp = " << resp;
		
		resp.clear();
		
		while(window.isOpen())
		{
			sf::Event event;
			while(window.pollEvent(event))
			{
				if(event.type == sf::Event::Closed)
					window.close();
				
				chess.handleMouseEvent(event, window);
			}
			
			window.clear();
			chess.draw(window);
			window.display();
		}
		
		std::cout << "\ndone.\n";
	}
	
	catch(const std::exception &e)
	{
		std::cerr << "error: " << e.what() << "\n";
		
		return -1;
	}
	
	return 0;
}
