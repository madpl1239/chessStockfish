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

std::string goNewGame(Stockfish& engine);
std::string getNextMove(Stockfish& engine, std::string& moves);
