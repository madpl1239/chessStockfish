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
#include "defines.hpp"
#include "utils.hpp"


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
					
					++index;
				}
			}
		}
	}

	void handleMouseEvent(sf::Event& event, sf::RenderWindow& window)
	{
		sf::Vector2i mousePos = sf::Mouse::getPosition(window);
		sf::Vector2i tile((mousePos.x - OFFSET) / TILE_SIZE, (mousePos.y - OFFSET) / TILE_SIZE);
		
		if(event.type == sf::Event::MouseButtonPressed and event.mouseButton.button == sf::Mouse::Left)
		{
			if(not m_pieceSelected)
			{
				// change of figure
				m_selectedPieceIndex = getPieceAt(tile);
				
				if(m_selectedPieceIndex != -1)
				{
					m_pieceSelected = true;
					m_selectedTile = tile;
					
					sf::Vector2f oldPos = m_pieces[m_selectedPieceIndex].getPosition();
					m_command = toChess(oldPos);
				}
			}
			else
			{
				// attempt to make a move
				if(isValidMove(m_selectedTile, tile))
				{
					m_pieces[m_selectedPieceIndex].setPosition(tile.x * TILE_SIZE + OFFSET,
															   tile.y * TILE_SIZE + OFFSET);
					m_selectedPieceIndex = -1;
					m_pieceSelected = false;
					
					sf::Vector2f newPos;
					newPos.x = mousePos.x - OFFSET;
					newPos.y = mousePos.y - OFFSET;
					m_command += toChess(newPos);
				}
			}
		}
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
	
	void move(std::string str, std::string& globalPos)
	{
		sf::Vector2f oldPos = toCoords(str[0], str[1]);
		sf::Vector2f newPos = toCoords(str[2], str[3]);
		
		for(int i = 0; i < 32; ++i)
			if(m_pieces[i].getPosition() == newPos)
				m_pieces[i].setPosition(-100, -100);
		
		for(int i = 0; i < 32; ++i)
			if(m_pieces[i].getPosition() == oldPos)
				m_pieces[i].setPosition(newPos);
		
		// castling if the king not moved yet
		if(str == "e1g1") // king's move
			if(globalPos.find("e1") == -1) 
				move("h1f1", globalPos); // rook's move
		
		if(str == "e8g8")
			if(globalPos.find("e8") == -1)
				move("h8f8", globalPos);
		
		if(str == "e1c1")
			if(globalPos.find("e1") == -1) 
				move("a1d1", globalPos);
		
		if(str == "e8c8") 
			if(globalPos.find("e8") == -1)
				move("a8d8", globalPos);
	}
	
	std::string getCommand() const
	{
		return m_command;
	}

private:
	std::string toChess(sf::Vector2f piece)
	{
		std::string str;
		
		str += static_cast<char>(97 + (piece.x / TILE_SIZE));
		str += static_cast<char>(8 - (piece.y / TILE_SIZE) + 49);
		
		return str;
	}

	sf::Vector2f toCoords(char a, char b)
	{
		int x = static_cast<int>(a) - 97;
		int y = 7 - static_cast<int>(b) + 49;
		
		return sf::Vector2f(x * TILE_SIZE, y * TILE_SIZE);
	}
	
	int getPieceAt(sf::Vector2i tile)
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

	bool isValidMove(sf::Vector2i start, sf::Vector2i end)
	{
		return start != end and end.x >= 0 and end.x < 8 and end.y >= 0 and end.y < 8;
	}
	
private:
	Stockfish& m_engine;
	sf::Texture m_boardTexture{};
	sf::Texture m_piecesTexture{};
	sf::Sprite m_boardSprite{};
	sf::Sprite m_pieces[32]{};					// maximum 32 figures
	std::map<std::string, int> m_figureMap{};	// mapping a figure to an atlas index
	std::vector<std::string> m_board{};			// position in FEN notation
	std::string m_command{};					// last command by player

	sf::Vector2i m_selectedTile{};			// selected position
	int m_selectedPieceIndex = -1;			// index of the selected figure (-1 means none)
	bool m_pieceSelected = false;			// has a figure been selected?
};
