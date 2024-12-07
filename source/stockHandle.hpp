/*
 * stockHandle.hpp
 * 
 * 02-12-2024 by madpl
 */
#pragma once

#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include "defines.hpp"
#include <thread>


class Stockfish
{
public:
	Stockfish(const std::string& stockfishPath)
	{
		if(pipe(m_stockfishInput) == -1 or pipe(m_stockfishOutput) == -1)
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
			// child process
			close(m_stockfishInput[1]);
			close(m_stockfishOutput[0]);
			
			if(dup2(m_stockfishInput[0], STDIN_FILENO) == -1 or
				dup2(m_stockfishOutput[1], STDOUT_FILENO) == -1)
			{
				perror("error while redirecting descriptors");
				exit(1);
			}
			
			close(m_stockfishInput[0]);
			close(m_stockfishOutput[1]);
			
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
			// parent process
			close(m_stockfishInput[0]);
			close(m_stockfishOutput[1]);
		}
		
		// stockfish readness check
		sendCommand("uci");
		std::string response = getResponse();
		if(response.find("uciok") == std::string::npos)
			throw std::runtime_error("stockfish did not respond correctly to UCI");
	}

	void sendCommand(const std::string& command)
	{
		std::string cmd = command + "\n";
		
		if(write(m_stockfishInput[1], cmd.c_str(), cmd.size()) == -1)
		{
			perror("error writing to stockfish");
			throw std::runtime_error("failed to send command to stockfish");
		}
	}

	std::string getResponse()
	{
		char buffer[256];
		std::string response;
		ssize_t bytesRead;
		
		while((bytesRead = read(m_stockfishOutput[0], buffer, sizeof(buffer) - 1)) > 0)
		{
			buffer[bytesRead] = '\0';
			response += buffer;
			
			if (response.find("uciok") != std::string::npos or 
				response.find("readyok") != std::string::npos or 
				response.find("bestmove") != std::string::npos)
			{
				break;
			}
		}
		
		if(bytesRead == -1)
		{
			perror("error reading from stockfish");
			throw std::runtime_error("could not read response from stockfish");
		}
		
		return response;
	}

	~Stockfish()
	{
		close(m_stockfishInput[1]);
		close(m_stockfishOutput[0]);
		
		// wait for stockfish process
		waitpid(m_pid, nullptr, 0);
	}
	
private:
	int m_stockfishInput[2];
	int m_stockfishOutput[2];
	pid_t m_pid;
};
