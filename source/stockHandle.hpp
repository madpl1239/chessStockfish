/*
 * stockHandle.hpp
 * 
 * 02-12-2024 by madpl
 */
#pragma once

#include "defines.hpp"

#ifdef LINUX

#include <thread>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>


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
		std::cout << "[DEBUG] ctor Stockfish...\n";
		#endif
	}

	~Stockfish()
	{
		close(m_pipe1[WRITE]);
		close(m_pipe2[READ]);
		
		// wait for stockfish process
		waitpid(m_pid, nullptr, 0);
		
		#ifdef DEBUG
		std::cout << "[DEBUG] dtor Stockfish...\n";
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
		char buffer[256];
		ssize_t bytesRead;
		std::string response;
		
		while((bytesRead = read(m_pipe2[READ], buffer, sizeof(buffer) - 1)) > 0)
		{
			buffer[bytesRead] = '\0';
			response += buffer;
			
			if(response.find("uciok") != std::string::npos or 
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
	
private:
	pid_t m_pid;

	// for parent process
	int m_pipe1[2];
	
	// for child process - stockfish
	int m_pipe2[2];
};

#endif // LINUX


#ifdef WINDOWS

#include <windows.h>
#include <string>
#include <stdexcept>
#include <iostream>
#include <thread>


class Stockfish
{
public:
	Stockfish(const std::string& stockfishPath)
	{
		SECURITY_ATTRIBUTES saAttr;
		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = NULL;

		if(!CreatePipe(&m_pipeReadChild, &m_pipeWriteParent, &saAttr, 0))
			throw std::runtime_error("CreatePipe failed");

		if(!SetHandleInformation(m_pipeWriteParent, HANDLE_FLAG_INHERIT, 0))
			throw std::runtime_error("SetHandleInformation failed");

		if(!CreatePipe(&m_pipeReadParent, &m_pipeWriteChild, &saAttr, 0))
			throw std::runtime_error("CreatePipe failed");

		if(!SetHandleInformation(m_pipeReadParent, HANDLE_FLAG_INHERIT, 0))
			throw std::runtime_error("SetHandleInformation failed");

		STARTUPINFO siStartInfo;
		ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
		siStartInfo.cb = sizeof(STARTUPINFO);
		siStartInfo.hStdError = m_pipeWriteChild;
		siStartInfo.hStdOutput = m_pipeWriteChild;
		siStartInfo.hStdInput = m_pipeReadChild;
		siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

		PROCESS_INFORMATION piProcInfo;
		ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

		if(!CreateProcess(stockfishPath.c_str(), NULL, NULL, NULL, TRUE,
						   0, NULL, NULL, &siStartInfo, &piProcInfo))
		{
			throw std::runtime_error("CreateProcess failed");
		}

		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);

		CloseHandle(m_pipeWriteChild);
		CloseHandle(m_pipeReadChild);

		sendCommand("uci");
		std::string response = getResponse();
		if(response.find("uciok") == std::string::npos)
			throw std::runtime_error("Stockfish did not respond correctly to UCI");
	}

	~Stockfish()
	{
		CloseHandle(m_pipeWriteParent);
		CloseHandle(m_pipeReadParent);
	}

	void sendCommand(const std::string& command)
	{
		DWORD written;
		std::string cmd = command + "\n";

		if(!WriteFile(m_pipeWriteParent, cmd.c_str(), cmd.size(), &written, NULL))
			throw std::runtime_error("Failed to send command to Stockfish");
	}

	std::string getResponse()
	{
		CHAR buffer[1024];
		DWORD bytesRead;
		std::string response;

		while(ReadFile(m_pipeReadParent, buffer, sizeof(buffer) - 1, &bytesRead, NULL) and bytesRead != 0)
		{
			buffer[bytesRead] = '\0';
			response += buffer;

			if(response.find("uciok") != std::string::npos or
				response.find("readyok") != std::string::npos or
				response.find("bestmove") != std::string::npos)
			{
				break;
			}
		}

		if(bytesRead == 0 && GetLastError() != ERROR_MORE_DATA)
			throw std::runtime_error("Failed to read response from Stockfish");

		#ifdef DEBUG
		std::cout << "[DEBUG] Full response: " << response << "\n";
		#endif

		return response;
	}

private:
	HANDLE m_pipeReadParent;
	HANDLE m_pipeWriteParent;
	HANDLE m_pipeReadChild;
	HANDLE m_pipeWriteChild;
};

#endif
