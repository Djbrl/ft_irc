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

//Read data from the clientSocket and return it as an object for processing, print an error message if the socket is closed or empty(ctrl+D)
std::istringstream  IrcServer::readData(int clientSocket)
{
	char    socketData[MAX_DATA_SIZE] = {0};
	int     bytes = 0;

	try
	{
		bytes = read(clientSocket, socketData, sizeof(socketData) - 1); 
		if (bytes == 0)
		{
			std::cout << Utils::getLocalTime() << "Client disconnected." << std::endl;
			g_clientSockets.erase(std::remove(g_clientSockets.begin(), g_clientSockets.end(), clientSocket), g_clientSockets.end());
			close(clientSocket);
			return std::istringstream("NULL");
		}
		if (bytes < 0)
		{
			throw ReadSocketException();
			return std::istringstream("NULL");
		}
	}
	catch(const ReadSocketException &e)
	{
		std::cerr << e.what() << '\n';
	}
	std::istringstream iss(socketData);
	return iss;
}

//Display the data received from the current connected client
void IrcServer::displayClientData(int clientSocket)
{
	char    socketData[MAX_DATA_SIZE] = {0};
	char    clientIP[INET_ADDRSTRLEN];
	int     bytes = 0;

	try
	{
		bytes = read(clientSocket, socketData, sizeof(socketData) - 1); 
		if (bytes == 0)
		{
			std::cout << Utils::getLocalTime() << "Client disconnected." << std::endl;
			g_clientSockets.erase(std::remove(g_clientSockets.begin(), g_clientSockets.end(), clientSocket), g_clientSockets.end());
			close(clientSocket);
			return ;
		}
		if (bytes < 0)
		{
			throw ReadSocketException();
			return ;
		}
		inet_ntop(AF_INET, &(_serverSockAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
		std::cout << Utils::getLocalTime() << "[" << Utils::trimBackline(socketData) << "] received from client[" << clientSocket << "] " << BYELLOW << clientIP << RESET << "." << std::endl;
	}
	catch(const ReadSocketException &e)
	{
		std::cerr << e.what() << '\n';
	}
}

//Process the data received from the current client
//NOT IMPLEMETED YET
void IrcServer::processCommand(std::istringstream &requestField, int clientSocket)
{
	std::vector<std::string>    arguments;
	std::string                 command;

	(void)clientSocket;
	requestField >> command;
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
