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


class Chess
{
public:
	Chess()
	{
		if(!m_boardTexture.loadFromFile("./resource/board.png"))
			throw std::runtime_error("Could not load board.png");
		
		if(!m_piecesTexture.loadFromFile("./resource/figures.png"))
			throw std::runtime_error("Could not load figures.png");
		
		m_boardSprite.setTexture(m_boardTexture);
		
		for(int i = 0; i < 32; ++i)
			m_pieces[i].setTexture(m_piecesTexture);
		
		m_figureMap = {{"R", 0}, {"N", 1}, {"B", 2}, {"Q", 3}, {"K", 4}, {"P", 5}};
	}

	void setupBoard()
	{
		m_board = {"rnbqkbnr", "pppppppp", "........", "........",
					"........", "........", "PPPPPPPP", "RNBQKBNR"};
		
		int index = 0;
		for(int y = 0; y < 8; ++y)
		{
			for(int x = 0; x < 8; ++x)
			{
				char piece = m_board[y][x];
				
				if(piece != '.')
				{
					// if piece is letter then color is 0
					int color = (piece >= 'a' and piece <= 'z') ? 0 : 1;
					int type = m_figureMap[std::string(1, std::toupper(piece))];
					
					m_pieces[index].setTextureRect(sf::IntRect(type * TILE_SIZE, color * TILE_SIZE, TILE_SIZE, TILE_SIZE));
					m_pieces[index].setPosition(x * TILE_SIZE + TILE_SIZE / 2, y * TILE_SIZE + TILE_SIZE / 2);
					m_pieces[index].setOrigin(0, 1);
					++index;
				}
			}
		}
	}

	void handleMouseEvent(sf::Event& event, sf::RenderWindow& window)
	{
		sf::Vector2i mousePos = sf::Mouse::getPosition(window);
		
		if(event.type == sf::Event::MouseButtonPressed and event.mouseButton.button == sf::Mouse::Left)
		{
			#ifdef DEBUG
			std::cout << "mouse pressed...\n";
			#endif
			
			for(int i = 0; i < 32; ++i)
			{
				if(m_pieces[i].getGlobalBounds().contains(mousePos.x, mousePos.y))
				{
					m_draggingPiece = &m_pieces[i];
					m_draggingStartPos = m_draggingPiece->getPosition();
					m_draggingBoardPos = sf::Vector2i(mousePos.x / TILE_SIZE, mousePos.y / TILE_SIZE);
					
					#ifdef DEBUG
					std::cout << "m_draggingBoardPos = ("
							  << m_draggingBoardPos.x << ", "
							  << m_draggingBoardPos.y << ")\n";
					#endif
					
					break;
				}
			}
		}
		
		if(event.type == sf::Event::MouseButtonReleased and event.mouseButton.button == sf::Mouse::Left)
		{
			if(m_draggingPiece)
			{
				sf::Vector2i dropPos = sf::Vector2i(mousePos.x / TILE_SIZE, mousePos.y / TILE_SIZE);
				
				if(isValidMove(m_draggingBoardPos, dropPos))
					m_draggingPiece->setPosition(dropPos.x * TILE_SIZE + TILE_SIZE / 2, dropPos.y * TILE_SIZE + TILE_SIZE / 2);
				else
					// incorrect move, reverse to old position
					m_draggingPiece->setPosition(m_draggingStartPos);
				
				m_draggingPiece = nullptr;
			}
			
			#ifdef DEBUG
			std::cout << "mouse released...\n";
			#endif
		}
		
		if(event.type == sf::Event::MouseMoved)
		{
			if(m_draggingPiece)
				m_draggingPiece->setPosition(mousePos.x, mousePos.y);
		}
	}

	sf::Sprite* getPieceAt(sf::Vector2i tile)
	{
		for(auto& piece : m_pieces)
		{
			if(piece.getGlobalBounds().contains(tile.x * TILE_SIZE, tile.y * TILE_SIZE))
				return &piece;
		}
		
		return nullptr;
	}

	std::string toUCI(sf::Vector2i start, sf::Vector2i end)
	{
		char startFile = 'a' + start.x;
		char startRank = '1' + (7 - start.y);
		char endFile = 'a' + end.x;
		char endRank = '1' + (7 - end.y);
		
		return std::string{startFile, startRank, endFile, endRank};
	}

	std::string extractBestMove(const std::string& response)
	{
		size_t pos = response.find("bestmove");
		
		if(pos != std::string::npos)
			return response.substr(pos + 9, 4);
		
		return "";
	}

	void executeMove(const std::string& moveUCI)
	{
		sf::Vector2f oldPos = toCoord(moveUCI[0], moveUCI[1]);
		sf::Vector2f newPos = toCoord(moveUCI[2], moveUCI[3]);
		
		for(auto& piece : m_pieces)
		{
			if(piece.getPosition() == newPos)
				piece.setPosition(-100, -100); // Captured piece
			
			if(piece.getPosition() == oldPos)
				piece.setPosition(newPos);
		}
	}

	void draw(sf::RenderWindow& window)
	{
		window.draw(m_boardSprite);
		
		for(auto& piece : m_pieces)
			window.draw(piece);
	}

	bool isValidMove(sf::Vector2i start, sf::Vector2i end)
	{
		return start != end and end.x >= 0 and end.x < 8 and end.y >= 0 and end.y < 8;
	}

private:
	sf::Vector2f toCoord(char file, char rank)
	{
		int x = file - 'a';
		int y = 7 - (rank - '1');
		
		return sf::Vector2f(x * TILE_SIZE + TILE_SIZE / 2, y * TILE_SIZE + TILE_SIZE / 2);
	}

	sf::Texture m_boardTexture;
	sf::Texture m_piecesTexture;
	sf::Sprite m_boardSprite;
	sf::Sprite m_pieces[32]; 				// Maximum 32 pieces
	std::map<std::string, int> m_figureMap; // Piece map to index atlas
	std::vector<std::string> m_board; 		// Position representation in UCI

	sf::Sprite* m_draggingPiece = nullptr;
	sf::Vector2f m_draggingStartPos;
	sf::Vector2i m_draggingBoardPos;
};
