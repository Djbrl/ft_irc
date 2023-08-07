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
	char welcomeMessage[512] = "\nWelcome to ft_IRC! Authenticate with PASS <password> or \"/quote PASS <password>\" if you're using IRSSI.\n\r\n";
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
		this->_ConnectedUsers.addUser(dataSocketFd);
	} catch (const AcceptException& e) {
		std::cerr << e.what() << '\n';
	}
	return dataSocketFd;
}

void IrcServer::addChannel(const std::string &channelName, User &owner)
{
	if (channelName.empty())
		return ;
	if (_Channels.find(channelName) == _Channels.end())
		_Channels[channelName] = Channel(channelName, owner);
}

void IrcServer::removeChannel(const std::string &channelName)
{
	if (channelName.empty())
		return ;
	if (_Channels.find(channelName) != _Channels.end())
		_Channels.erase(channelName);
}

std::string     IrcServer::parsePassCommand(int clientFd, std::stringstream &commandCopy) {

	std::vector<std::string>    wordSplit;
	std::string                 word;

	(void)clientFd;

	while(commandCopy >> word)
		wordSplit.push_back(word);
	//printing vector to test
	// for (std::size_t i = 0; i < wordSplit.size(); i++)
	//     std::cout << wordSplit[i] << std::endl;
	if (wordSplit.size() == 0)
		std::cout << "Password missing !" << std::endl;
	for (std::size_t i = 0; i < wordSplit.size(); i++)
	{
		if ((Utils::isEven(i) && wordSplit[i] == PASS) || (Utils::isOdd(i) && wordSplit[i] != PASS))
		{
			std::cerr << "Error : SOMETHING WRONG WITH PASSWORD." << std::endl;
			return ("");
		}
	}
	return (word);
}

void    IrcServer::passCommand(int clientFd, std::string passCommand, std::stringstream &commandCopy)
{
		std::string clientPassword;
		(void)clientFd;
		
		//if the client is NOT already connected {
			if (passCommand.find(USER) != std::string::npos || passCommand.find(NICK) != std::string::npos
				|| passCommand.find(INVITE) != std::string::npos || passCommand.find(MODE) != std::string::npos 
				|| passCommand.find(TOPIC) != std::string::npos || passCommand.find(KICK) != std::string::npos)
			{
				std::cerr << "Error : There cannot be different commands." << std::endl;
				return ;
			}
			if (!(clientPassword = parsePassCommand(clientFd, commandCopy)).empty())
			{
				if (clientPassword == this->_serverPassword)
				{
					//add user to connected users
					//set user authenticated status to true
					std::cout << "PASSWORD OK !" << std::endl;
				}
				else
				{
					std::cerr << "Error : SOMETHING WRONG WITH PASSWORD." << std::endl;
					return ;
				}
			}
		// }
		//else
		// {
			//Tell client he is already authentified ?
		// }
}

void    IrcServer::dispatchQuery(int clientFd, std::string clientQuery) {

	std::stringstream   queryCopy(clientQuery);
	std::string         command;
	(void)clientFd;
	
	queryCopy >> command;

	// if(client is not authentified && command.compare(PASS) != 0)
		//tell him that authentification is needed
	if (command.compare(PASS) == 0)
		passCommand(clientFd, clientQuery, queryCopy);
	// else if (command.compare(USER) == 0)
	//     userCommand(queryCopy);
	// else if (command.compare(NICK) == 0)
	//     nickCommand(queryCopy);
	// else if (command.compare(INVITE) == 0)
	//     inviteCommand(queryCopy);
	// else if (command.compare(MODE) == 0)
	//     modeCommand(queryCopy);
	// else if (command.compare(TOPIC) == 0)
	//     topicCommand(queryCopy);
	// else if (command.compare(KICK) == 0)
	//     kickCommand(queryCopy);
	// else
	// {
	//     handle whatever we have to handle if it is none of the above !
	// }

}

void	IrcServer::parseQuery(int clientFd, std::string clientQuery) {

	std::size_t queryLen = clientQuery.size();
	std::size_t start = 0;
	std::size_t end, found;
	std::string subQuery;

	//check basics
	if (queryLen > 512)
		std::cerr << " Error : Client's request is too long. " << std::endl;
	if (clientQuery[queryLen - 2] != '\r' || clientQuery[queryLen - 1] != '\n')
		std::cerr << "Error : Client's request does not end with the appropriate characters." << std::endl;
	//ignore spaces
	while (clientQuery[start] == ' ')
		start++;
	found = clientQuery.find("\r\n");
	while(found != std::string::npos)
	{
		end = found;
		//ignore spaces
		if (clientQuery[found - 1] == ' ')
		{
			end = found;
			while (clientQuery[end - 1] == ' ')
				end--;
		}
		//extract each command until the next "\r\n"
		subQuery = clientQuery.substr(start, end - start);
		std::cout << "Substring : " << " [" <<subQuery << "]" << std::endl;

		//implement the parsing logic for each command individually but dispatch them first
		dispatchQuery(clientFd, subQuery);

		//update start to analyze the rest of the query
		start = found + 2;
		//ignore spaces
		while(clientQuery[start] == ' ')
			start++;
		//find the next "\r\n" occurrence
		found = clientQuery.find("\r\n", start);
	}

}

void IrcServer::printSocketData(int clientSocket, char* socketData)
{
	std::string	clientIP;

	clientIP = inet_ntoa(_serverSockAddr.sin_addr);
	std::cout << Utils::getLocalTime() << "[" << Utils::trimBackline(socketData) << "] received from client[" << clientSocket << "] " << BYELLOW << clientIP << RESET << "." << std::endl;
	return;
}