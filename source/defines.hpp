/*
 * defines.hpp
 *
 * 02-12-2024 by madpl
 */ 
#pragma once

// for debugging
#define DEBUG 1
// #undef DEBUG

// for Linux, Windows Systems
#define LINUX
#undef WINDOWS

#define READ 0
#define WRITE 1

const int BOARD_SIZE = 504; // size of chessBoard (504x504 pikseli)
const int TILE_SIZE = 56;   // chess square size
const int OFFSET = 28;		// offset for piece
