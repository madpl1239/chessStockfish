/*
 * utils.hpp 
 * 
 * Declaration of utility functions.
 * 
 * 03-01-2025 by madpl
 */
#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include "defines.hpp"
#include "stockHandle.hpp"

std::string toChess(sf::Vector2f piece);
sf::Vector2f toCoords(char a, char b);
void move(sf::Sprite f[], std::string str, std::string& globalPos);
std::string goNewGame(Stockfish& engine);
std::string getNextMove(Stockfish& engine, std::string& moves);
