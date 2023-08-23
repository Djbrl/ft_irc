#include "IrcServer.hpp"

//MESSAGES___________________________________________________________________________________________________________

std::vector<std::string> IrcServer::splitStringByCRLF(const std::string &socketData, char *buffer)
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
	memset(buffer, '\0', sizeof(*buffer) * MESSAGE_BUFFER_SIZE);
    if (startPos < socketData.length())
	{
        std::string last_in_buffer = socketData.substr(startPos);
		memcpy(buffer, last_in_buffer.c_str(), std::min(size_t(MESSAGE_BUFFER_SIZE), last_in_buffer.size()));
    }
    return result;
}

void    IrcServer::safeSendMessage(int clientFd, std::string message)
{
	int 		bytes;
	int 		dataSent = 0;

	std::string nickname = "";
	User *user = _ConnectedUsers.getUser(clientFd);
	if (user != NULL)
		nickname = user->getNickname();
	// std::cout << "SafeSend: fd[" << clientFd << "] nick[" << nickname << "] len[" << message.size() << "] message[" << message << "]" << std::endl;
	
	if (message.size() > 512) {
		std::cout << Utils::getLocalTime() << "Warning : Message sent is more than 512 characters, truncating the result" << std::endl;
		message = message.substr(0, 510);
		message += "\r\n";
		std::cout << Utils::getLocalTime() << "Warning : message is now [" << message << "]" << std::endl;
	}
	
	//send message
	while (size_t(dataSent) < message.size())
	{
		if ((bytes = send(clientFd, message.c_str() + dataSent, message.size() - dataSent, MSG_DONTWAIT)) <= 0)
		{
			std::cerr << "Error : Couldn't send message [" << message << "...] to client [" << clientFd << "]" << std::endl;
			disconnectUserFromServer(clientFd);
			return ;
		}
		dataSent += bytes;
	}
	return ;
}

void    IrcServer::sendWelcomeMessage(int clientSocket)
{
	char welcomeMessage[512] = "You have successfully connected to ft_IRC! Authenticate with PASS <password> or \"/quote PASS <password>\" if you're using IRSSI.\n\r\n";
	safeSendMessage(clientSocket, welcomeMessage);
};

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
	std::map<std::string, Channel>::iterator it = _Channels.begin();
	if (channelName.empty())
		return ;

	while (it != _Channels.end())
	{
		if (it->first == channelName)
			return ;
		it++;
	}
	_Channels[channelName] = Channel(channelName, owner);
}

void    IrcServer::removeChannel(const std::string &channelName)
{
	if (channelName.empty())
		return ;
	if (_Channels.find(channelName) != _Channels.end())
		_Channels.erase(channelName);
}

void	IrcServer::updateMemberInChannels(std::string &oldNick, User &target)
{
	std::map<std::string, Channel>::iterator it = _Channels.begin();

	while (it != _Channels.end())
	{
		it->second.updateMemberNickname(oldNick, target);
		it++;
	}
}

//CLEAN UP___________________________________________________________________________________________________________

void	IrcServer::transferOwnership(Channel &chan)
{
	if (chan.getOperatorsList().size() > 1)
	{
		//first operator is always owner, so delegate to 2nd operator
		User	newOwner = chan.getOperatorsList()[1];
		chan.setChannelOwner(newOwner);
		chan.sendMessageToUsers("ownership of the channel has been transfered to @" + newOwner.getNickname(), "server");
		std::string notice = "NOTICE " + chan.getChannelName() + " You are now the channel owner.\r\n";
		safeSendMessage(newOwner.getSocket(), notice);
	}
	else
	{
		std::string notice = "NOTICE " + chan.getChannelName() + " Owner left without a successor, this channel is now headless.\r\n";
		chan.sendNoticeToUsers(notice, "server");
	}
}

void    IrcServer::disconnectUserFromServer(int clientFd)
{
	int j = 0;
	std::map<std::string, Channel>::iterator	it = _Channels.begin();
	User										*userToRemove = _ConnectedUsers.getUser(clientFd);
	
	//REMOVE USER FROM ALL CHANNELS AND USERMAP
	if (userToRemove != NULL)
	{
		if (*userToRemove == it->second.getChannelOwner())
			transferOwnership(it->second);
		while (it != _Channels.end())
		{
			if (it->second.hasMember(*userToRemove))
				it->second.removeMember(*userToRemove);
			if (it->second.isChannelOp(*userToRemove))
				it->second.removeOperator(*userToRemove);
			it++;
		}
		_ConnectedUsers.removeUser(clientFd);
	}
	//REMOVE FROM FD LISTS
	for (unsigned int i = 0; i < g_clientSockets.size(); i++)
	{
		if (clientFd == g_clientSockets[i])
		{
			FD_CLR(clientFd, &_clientsFdSet);
			g_clientSockets.erase(g_clientSockets.begin() + j);
			close(clientFd);
			return ;
		}
		j++;
	}
}