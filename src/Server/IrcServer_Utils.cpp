#include "IrcServer.hpp"

//MESSAGES___________________________________________________________________________________________________________

std::vector<std::string> IrcServer::splitStringByCRLF(const std::string &socketData)
{
    std::vector<std::string> result;

    size_t startPos = 0;
    size_t delimiterPos = socketData.find("\r\n");

    while (delimiterPos != std::string::npos)
	{
        result.push_back(socketData.substr(startPos, delimiterPos - startPos));
        startPos = delimiterPos + 2;
        delimiterPos = socketData.find("\r\n", startPos);
    }
    if (startPos < socketData.length())
	{
        result.push_back(socketData.substr(startPos));
    }
    return result;
}

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

void    IrcServer::sendWelcomeMessage(int clientSocket)
{
	char welcomeMessage[512] = "Welcome to ft_IRC! Authenticate with PASS <password> or \"/quote PASS <password>\" if you're using IRSSI.\n\r\n";
	safeSendMessage(clientSocket, welcomeMessage);
};

void    IrcServer::sendServerResponse(int clientFd, char *message)
{
	for (unsigned int i = 0; i < g_clientSockets.size(); i++)
	{
		if (g_clientSockets[i] == clientFd)
			_serverResponses[g_clientSockets[i]] = message;
	}
}

void    IrcServer::printSocketData(int clientSocket, char* socketData)
{
	std::string	clientIP;

	clientIP = inet_ntoa(_serverSockAddr.sin_addr);
	std::cout << Utils::getLocalTime() << "[" << Utils::trimBackline(socketData) << "] received from client[" << clientSocket << "] " << BYELLOW << clientIP << RESET << "." << std::endl;
	return;
}

//CHANNEL___________________________________________________________________________________________________________

void    IrcServer::addChannel(const std::string &channelName, User &owner)
{
	if (channelName.empty())
		return ;
	if (_Channels.find(channelName) == _Channels.end())
		_Channels[channelName] = Channel(channelName, owner);
}

void    IrcServer::removeChannel(const std::string &channelName)
{
	if (channelName.empty())
		return ;
	if (_Channels.find(channelName) != _Channels.end())
		_Channels.erase(channelName);
}

//CLEAN UP___________________________________________________________________________________________________________

void    IrcServer::clearFdFromList(int clientFd)
{
	int j = 0;
	for (unsigned int i = 0; i < g_clientSockets.size(); i++)
	{
		if (clientFd == g_clientSockets[i])
		{
			FD_CLR(clientFd, &_clientsFdSet);
			g_clientSockets.erase(g_clientSockets.begin() + j);
			_serverResponses.erase(clientFd);
			this->_ConnectedUsers.removeUser(clientFd);
			close(clientFd);
			return ;
		}
		j++;
	}
}