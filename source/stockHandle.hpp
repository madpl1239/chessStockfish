/*
 * stockHandle.hpp
 * 
 * 02-12-2024 by madpl
 */
#pragma once

#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <thread>
#include "defines.hpp"


class Stockfish
{
public:
	Stockfish(const std::string& stockfishPath)
	{
		if(pipe(m_pipe1) == -1 or pipe(m_pipe2) == -1)
		{
			perror("error creating streams");
			throw std::runtime_error("failed to create streams");
		}
		
		m_pid = fork();
		if(m_pid == -1)
		{
			perror("error during fork()");
			throw std::runtime_error("failed to create child process");
		}
		
		if(m_pid == 0)
		{
			// child process - stockfish
			close(m_pipe1[WRITE]);
			close(m_pipe2[READ]);
			
			if(dup2(m_pipe1[READ], STDIN_FILENO) == -1 or
				dup2(m_pipe2[WRITE], STDOUT_FILENO) == -1)
			{
				perror("error while redirecting descriptors");
				exit(1);
			}
			
			close(m_pipe1[READ]);
			close(m_pipe2[WRITE]);
			
			if(access(stockfishPath.c_str(), X_OK) == -1)
			{
				perror("Stockfish does not exist or permissions not available");
				exit(1);
			}
			
			execlp(stockfishPath.c_str(), stockfishPath.c_str(), nullptr);
			perror("error starting Stockfish");
			exit(1);
		}
		else
		{
			// parent process - main function
			close(m_pipe1[READ]);
			close(m_pipe2[WRITE]);
		}
		
		// stockfish readness check
		sendCommand("uci");
		std::string response = getResponse();
		if(response.find("uciok") == std::string::npos)
			throw std::runtime_error("stockfish did not respond correctly to UCI");
		
		#ifdef DEBUG
		std::cout << "ctor Stockfish...\n";
		#endif
	}

	~Stockfish()
	{
		close(m_pipe1[WRITE]);
		close(m_pipe2[READ]);
		
		// wait for stockfish process
		waitpid(m_pid, nullptr, 0);
		
		#ifdef DEBUG
		std::cout << "dtor Stockfish...\n";
		#endif
	}

	void sendCommand(const std::string& command)
	{
		std::string cmd = command + "\n";
		
		if(write(m_pipe1[WRITE], cmd.c_str(), cmd.size()) == -1)
		{
			perror("error writing to stockfish");
			throw std::runtime_error("failed to send command to stockfish");
		}
	}

	std::string getResponse()
	{
		char buffer[1024];  // Zwiększ rozmiar bufora
		ssize_t bytesRead;
		std::string response;
		
		while((bytesRead = read(m_pipe2[READ], buffer, sizeof(buffer) - 1)) > 0)
		{
			buffer[bytesRead] = '\0';
			response += buffer;
			
			// Zakończ, jeśli odpowiedź zawiera kluczowe słowa
			if(response.find("uciok") != std::string::npos or 
				response.find("readyok") != std::string::npos or 
				response.find("bestmove") != std::string::npos)
			{
				break;
			}
		}
		
		if(bytesRead == -1)
			throw std::runtime_error("could not read response from stockfish");
		
		// Logowanie pełnej odpowiedzi
		std::cout << "[DEBUG] Full response: " << response << std::endl;
		
		return response;
	}
	
private:
	pid_t m_pid;

	// for parent process
	int m_pipe1[2];
	
	// for child process - stockfish
	int m_pipe2[2];
};
