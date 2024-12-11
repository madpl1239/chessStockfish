/*
 * main_stock.cpp
 *
 * Testing sockHandle class.
 * 
 * 06-12-2024 by madpl
 */
#include <iostream>
#include "stockHandle.hpp"


int main(void)
{
	Stockfish engine("stockfish");
	
	engine.sendCommand("uci");
	std::string resp = engine.getResponse();
	
	std::cout << "resp.size() = " << resp.size() << "\n";
	std::cout << "resp = " << resp << "\n";
	
	resp.clear();
	engine.sendCommand("isready");
	resp = engine.getResponse();
	
	std::cout << "resp.size() = " << resp.size() << "\n";
	std::cout << "resp = " << resp << "\n";
	
	resp.clear();
	
	std::cout << "\ndone.\n";
	
	return 0;
}
