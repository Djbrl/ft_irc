#include "IrcServer.hpp"

void    IrcServer::safeSendMessage(int clientFd, char *message)
{
	int bytes;
	int dataSent = 0;
	int messageLen = strlen(message); //length of the message to be sent

	while (dataSent < messageLen)
	{
		if ((bytes = send(clientFd, message + dataSent, messageLen - dataSent, MSG_DONTWAIT)) <= 0)
		{
			std::cout << "Error : Couldn't send data to client." << std::endl;
			return ;
		}
		dataSent += bytes;
	}
	return ;
}

void	IrcServer::sendWelcomeMessage(int clientSocket)
{
	char welcomeMessage[512] = "Welcome to ft_IRC! This server requires a password. Authenticate with PASS <password> or \"/quote PASS <password>\" if you're using IRSSI.\r\n";
	safeSendMessage(clientSocket, welcomeMessage);
};

void IrcServer::sendServerResponse(int clientFd, char *message)
{
	for (unsigned int i = 0; i < g_clientSockets.size(); i++)
	{
		if (g_clientSockets[i] == clientFd)
			_serverResponses[g_clientSockets[i]] = message;
	}
}

int IrcServer::acceptClient()
{
	std::string	clientIP;
	sockaddr	clientSockAddr;
	int			clientAddrLen;
	int			dataSocketFd;

	try
	{
		clientAddrLen = sizeof(_serverSockAddr);
		if ((dataSocketFd = accept(_serverFd, (struct sockaddr*)&clientSockAddr, (socklen_t*)&clientAddrLen)) == -1)
			throw AcceptException();
		FD_SET(dataSocketFd, &_clientsFdSet);
		g_clientSockets.push_back(dataSocketFd);
		clientIP = inet_ntoa(((struct sockaddr_in*)&clientSockAddr)->sin_addr);
		std::cout << Utils::getLocalTime() << "New client connection: [" << dataSocketFd << "] - " << BYELLOW << clientIP << RESET << "." << std::endl;
		sendWelcomeMessage(dataSocketFd);
		//CREATE A USER AND PUT IT IN LOBBYMAP UNTIL HE GIVES PASS AND NICK
		this->_ConnectedUsers.addUser(dataSocketFd);
	} catch (const AcceptException& e) {
		std::cerr << e.what() << '\n';
	}
	return dataSocketFd;
}

void IrcServer::printSocketData(int clientSocket, char* socketData)
{
	std::string	clientIP;

	clientIP = inet_ntoa(_serverSockAddr.sin_addr);
	std::cout << Utils::getLocalTime() << "[" << socketData << "] received from client[" << clientSocket << "] " << BYELLOW << clientIP << RESET << "." << std::endl;
	return;
}