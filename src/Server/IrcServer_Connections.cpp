#include "IrcServer.hpp"

//Accept connections from clients on the serverFd and print a message with the clientFD and the IP
int IrcServer::acceptClient()
{
	int     dataSocketFd;
	int     dataAddrLen;
	char    clientIP[INET_ADDRSTRLEN];

	try
	{
		dataAddrLen = sizeof(_serverSockAddr);
		if ((dataSocketFd = accept(_serverFd, (struct sockaddr *)&_serverSockAddr, (socklen_t *)&dataAddrLen)) == -1)
		{
			throw AcceptException();
			return -1;
		}
		g_clientSockets.push_back(dataSocketFd);
		inet_ntop(AF_INET, &(_serverSockAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
		std::cout << Utils::getLocalTime() << "New client connection : [" << dataSocketFd << "] - " << BYELLOW << clientIP << RESET << "." << std::endl;
	}
	catch(const AcceptException &e)
	{
		std::cerr << e.what() << '\n';
	}
	return dataSocketFd;
}

std::vector<std::string> IrcServer::readData(int clientSocket)
{
	std::vector<std::string> request;
	std::string word;
	char socketData[MAX_DATA_SIZE] = {0};
	int bytes = 0;

	try
	{
		bytes = read(clientSocket, socketData, sizeof(socketData) - 1);
		if (bytes == 0)
		{
			std::cout << Utils::getLocalTime() << "Client disconnected." << std::endl;
			std::vector<int>::iterator it = std::find(g_clientSockets.begin(), g_clientSockets.end(), clientSocket);
			if (it != g_clientSockets.end())
				g_clientSockets.erase(it);
			close(clientSocket);
			return std::vector<std::string>(1, "NULL");
		}
		if (bytes < 0)
		{
			throw ReadSocketException();
			return std::vector<std::string>(1, "NULL");
		}
	}
	catch (const ReadSocketException &e)
	{
		std::cerr << e.what() << '\n';
	}

	// Copy the string into a sstream
	std::stringstream iss(socketData);
	while (iss >> word)
	{
		request.push_back(word);
	}

	// Return the request as a vector of string
	return request;
}

void IrcServer::displayClientData(int clientSocket)
{
	char socketData[MAX_DATA_SIZE] = {0};
	char clientIP[INET_ADDRSTRLEN];
	int bytes = 0;

	try
	{
		bytes = read(clientSocket, socketData, sizeof(socketData) - 1);
		if (bytes == 0)
		{
			std::cout << Utils::getLocalTime() << "Client disconnected." << std::endl;
			std::vector<int>::iterator it = std::find(g_clientSockets.begin(), g_clientSockets.end(), clientSocket);
			if (it != g_clientSockets.end())
				g_clientSockets.erase(it);
			close(clientSocket);
			return;
		}
		if (bytes < 0)
		{
			throw ReadSocketException();
			return;
		}
		inet_ntop(AF_INET, &(_serverSockAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
		std::cout << Utils::getLocalTime() << "[" << Utils::trimBackline(socketData) << "] received from client[" << clientSocket << "] " << BYELLOW << clientIP << RESET << "." << std::endl;
	}
	catch (const ReadSocketException &e)
	{
		std::cerr << e.what() << '\n';
	}
}

//Process the data received from the current client
//NOT IMPLEMETED YET
void IrcServer::processCommand(std::vector<std::string> &requestField, int clientSocket)
{
	std::string command;

	(void)clientSocket;
	command = requestField[0];
	if (command == "NULL")
		return ;
	if (command == "NICK")
	{
		//do something
	}
	else if (command == "/exit")
	{
		//do something
	}
	else
	{
		//do something
	}
}
