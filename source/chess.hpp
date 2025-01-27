/*
 * chess.hpp
 * 
 * 05-12-2024 by madpl
 */
#pragma once

#include <SFML/Graphics.hpp>
#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <cctype>
#include <cmath>
#include <sstream>
#include "defines.hpp"


class ChessPiece : public sf::Sprite
{
public:
	ChessPiece():
		isWhite(false)
	{
		// empty
	}

public:
	bool isWhite;
};


class Chess
{
public:
	Chess(Stockfish& engine):
		m_engine(engine)
	{
		if(not m_boardTexture.loadFromFile("./resource/board.png"))
			throw std::runtime_error("Could not load board.png");
		
		if(not m_piecesTexture.loadFromFile("./resource/figures.png"))
			throw std::runtime_error("Could not load figures.png");
		
		m_boardSprite.setTexture(m_boardTexture);
		
		// initialization of pieces
		for(int i = 0; i < 32; ++i)
		{
			m_pieces[i].setTexture(m_piecesTexture);
			m_pieces[i].setOrigin(0, 1);
		}
		
		m_figureMap = {{"R", 0}, {"N", 1}, {"B", 2}, {"Q", 3}, {"K", 4}, {"P", 5}};
	}

	void setupBoard()
	{
		m_board = {
			"rnbqkbnr", "pppppppp", "........", "........",
			"........", "........", "PPPPPPPP", "RNBQKBNR"
		};
		
		int index = 0;
		for(int y = 0; y < 8; ++y)
		{
			for(int x = 0; x < 8; ++x)
			{
				char piece = m_board[y][x];
				if(piece != '.')
				{
					int type = m_figureMap[std::string(1, std::toupper(piece))];
					int color = (piece >= 'a' and piece <= 'z') ? 0 : 1;
					
					m_pieces[index].setTextureRect(sf::IntRect(type * TILE_SIZE, color * TILE_SIZE,
															   TILE_SIZE, TILE_SIZE));
					
					m_pieces[index].setPosition(x * TILE_SIZE + OFFSET, y * TILE_SIZE + OFFSET);
					
					m_pieces[index].isWhite = (color == 1);
					
					++index;
				}
			}
		}
	}

	void handleMouseEvent(sf::Event& event, sf::RenderWindow& window)
	{
		sf::Vector2i mousePos = sf::Mouse::getPosition(window);
		int mPosX = std::round((mousePos.x - OFFSET) / TILE_SIZE);
		int mPosY = std::round((mousePos.y - OFFSET) / TILE_SIZE);
		sf::Vector2f tile(mPosX, mPosY);

		if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
		{
			if(m_pieceSelected)
			{
				// if the user clicks on the selected piece again, deselect it
				if(tile == m_selectedTile)
				{
					m_pieceSelected = false;
					m_selectedPieceIndex = -1;
				}

				else if(isValidMove(m_selectedTile, tile))
				{
					// we're trying to make a move
					sf::Vector2f newPos(tile.x * TILE_SIZE + OFFSET, tile.y * TILE_SIZE + OFFSET);
					m_selectedPieceIndex = -1;
					m_pieceSelected = false;
					m_command += toChess(newPos);
					move(m_command);
					s_positions += " " + m_command;
					m_command.clear();
					m_stockfishMove.clear();

					// move of engine
					if(getNextMove())
					{
						move(m_stockfishMove);
						s_positions += " " + m_stockfishMove;
						m_command.clear();
						m_stockfishMove.clear();
					}
				}
			}
			else
			{
				// if no pawn is marked, try marking a pawn
				m_selectedPieceIndex = getPieceAt(tile);
				if(m_selectedPieceIndex != -1)
				{
					m_pieceSelected = true;
					m_selectedTile = tile;
					sf::Vector2f oldPos = m_pieces[m_selectedPieceIndex].getPosition();
					m_command = toChess(oldPos);
				}
			}
		}
	}

	std::string goNewGame()
	{
		m_engine.sendCommand("ucinewgame");
		m_engine.sendCommand("isready");
		
		std::string str = m_engine.getResponse();
		if(str.find("readyok") == std::string::npos)
			return "response error";
		
		#ifdef DEBUG
		std::cout << "[DEBUG] ucinewgame\n";
		std::cout << "[DEBUG] readyok\n";
		#endif	
		
		return "ok";
	}

	bool getNextMove()
	{
		// sending full movement history
		std::string command = "position startpos moves" + s_positions;
		m_engine.sendCommand(command);
		
		m_engine.sendCommand("go depth 2");
		std::string response = m_engine.getResponse();
		
		// extracting the 'bestmove' movement
		size_t bestmoveIdx = response.find("bestmove");
		if(bestmoveIdx != std::string::npos)
		{
			size_t endIdx = response.find(' ', bestmoveIdx + 9);
			m_stockfishMove = (endIdx != std::string::npos) 
								? response.substr(bestmoveIdx + 9, endIdx - (bestmoveIdx + 9))
								: response.substr(bestmoveIdx + 9);
			
			return true;
		}
		
		return false;
	}

	void move(std::string str)
	{
		auto arePositionsEqual = [](const sf::Vector2f& pos1, const sf::Vector2f& pos2, float epsilon = 0.5f)
		{
			return std::fabs(pos1.x - pos2.x) < epsilon and std::fabs(pos1.y - pos2.y) < epsilon;
		};
		
		sf::Vector2f oldPos = toCoords(str[0], str[1]);
		sf::Vector2f newPos = toCoords(str[2], str[3]);
		
		int movingPieceIndex = -1;
		
		// find and update 'oldPos' figure position
		for(int i = 0; i < 32; ++i)
		{
			if(arePositionsEqual(m_pieces[i].getPosition(), oldPos))
			{
				m_pieces[i].setPosition(newPos);
				
				// remember the index of the moved figure
				movingPieceIndex = i;
				
				break;
			}
		}
		
		if(movingPieceIndex == -1)
		{
			std::cout << "[ERROR] No piece found at oldPos (" << oldPos.x << ", " << oldPos.y << ")\n";
			
			return;
		}
		
		castling(str);

		// support for beating figures
		for(int i = 0; i < 32; ++i)
		{
			if(arePositionsEqual(m_pieces[i].getPosition(), newPos) and i != movingPieceIndex)
			{
				// check if it's an opponent's piece
				if(m_pieces[i].isWhite != m_pieces[movingPieceIndex].isWhite)
				{
					// hide captured pawn
					m_pieces[i].setPosition(-200, -200);
				}
				
				break;
			}
		}

		if(checkForCheck())
			std::cout << "The kings are in shah!\n";

		if(checkForMate())
			std::cout << "game over, mate!\n";
	}

	void draw(sf::RenderWindow& window)
	{
		window.draw(m_boardSprite);
		
		for(auto& piece : m_pieces)
			window.draw(piece);
		
		// highlight selected figure (if selected)
		if(m_pieceSelected and m_selectedPieceIndex != -1)
		{
			sf::RectangleShape highlight(sf::Vector2f(TILE_SIZE, TILE_SIZE));
			
			highlight.setFillColor(sf::Color(0x00, 0x80, 0x00, 0x80));
			highlight.setPosition(m_selectedTile.x * TILE_SIZE + OFFSET,
								  m_selectedTile.y * TILE_SIZE + OFFSET);
			
			window.draw(highlight);
		}
	}

private:
	std::string toChess(sf::Vector2f piece)
	{
		std::string str;
		
		str += static_cast<char>(97 + (piece.x / TILE_SIZE));
		str += static_cast<char>(8 - (piece.y / TILE_SIZE) + 49);
		
		return str;
	}

	sf::Vector2f toCoords(char col, char row)
	{
		int x = static_cast<int>(col - 'a');
		int y = 8 - static_cast<int>(row - '1') - 1;
		
		return sf::Vector2f(x * TILE_SIZE + OFFSET, y * TILE_SIZE + OFFSET);
	}

	int getPieceAt(sf::Vector2f tile)
	{
		for(int i = 0; i < 32; ++i)
		{
			if(m_pieces[i].getGlobalBounds().contains(tile.x * TILE_SIZE + OFFSET,
														tile.y * TILE_SIZE + OFFSET))
			{
				return i;
			}
		}
		
		return -1;
	}

	void castling(std::string& str)
	{
		// castling if the king not moved yet
		if(str == "e1g1") // king's move
			if(s_positions.find("e1") == -1) 
				move("h1f1"); // rook's move
		
		if(str == "e8g8")
			if(s_positions.find("e8") == -1)
				move("h8f8");
		
		if(str == "e1c1")
			if(s_positions.find("e1") == -1) 
				move("a1d1");
		
		if(str == "e8c8") 
			if(s_positions.find("e8") == -1)
				move("a8d8");
	}

	bool checkForCheck()
	{
		std::string command = "position startpos moves" + s_positions;
		m_engine.sendCommand(command);
		m_engine.sendCommand("go depth 1");
		std::string response = m_engine.getResponse();
		
		// Debugowanie odpowiedzi z silnika
		std::cout << "Response for check detection: " << response << std::endl;
		
		if(response.find("bestmove") != std::string::npos)
		{
			std::istringstream iss(response);
			std::string line;
			while(std::getline(iss, line))
			{
				// Sprawdzanie obecności frazy "info depth 1" oraz braku "mate" i "cp"
				if(line.find("info depth 1") != std::string::npos)
				{
					if(line.find("check") == std::string::npos and line.find("cp") == std::string::npos)
					{
						return true;
					}
				}
			}
		}
		
		return false;
	}

	bool checkForMate()
	{
		std::string command = "position startpos moves" + s_positions;
		m_engine.sendCommand(command);
		m_engine.sendCommand("go depth 1");
		std::string response = m_engine.getResponse();
		
		// Debugowanie odpowiedzi z silnika
		std::cout << "Response for mate detection: " << response << std::endl;
		
		if(response.find("mate") != std::string::npos)
			return true;
		
		return false;
	}

	bool isValidMove(sf::Vector2f start, sf::Vector2f end)
	{
		return start != end and end.x >= 0 and end.x < 8 and end.y >= 0 and end.y < 8;
	}

private:
	Stockfish& m_engine;
	sf::Texture m_boardTexture{};
	sf::Texture m_piecesTexture{};
	sf::Sprite m_boardSprite{};
	ChessPiece m_pieces[32]{};					// maximum 32 figures
	std::map<std::string, int> m_figureMap{};	// mapping a figure to an atlas index
	std::vector<std::string> m_board{};			// position in FEN notation
	std::string m_command{};					// last command by player
	std::string m_stockfishMove{};				// last respond from engine
	inline static std::string s_positions{};	// all positions for uci

	sf::Vector2f m_selectedTile{};	// selected position
	int m_selectedPieceIndex = -1;	// index of the selected figure (-1 means none)
	bool m_pieceSelected = false;	// has a figure been selected?
};
