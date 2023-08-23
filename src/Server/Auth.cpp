#include "IrcServer.hpp"

void	IrcServer::capls(std::vector<std::string> &requestArguments, User &currentClient)
{
	(void)requestArguments;
	std::string CAPLS = "CAP * LS :PASS NICK USER JOIN PART LIST PRIVMSG NOTICE MODE KICK INVITE TOPIC PING QUIT\r\n";
	safeSendMessage(currentClient.getSocket(), CAPLS);	
}

void	IrcServer::quit(std::vector<std::string> &requestArguments, User &currentClient)
{
	std::map<std::string, Channel>::iterator	it = _Channels.begin();
	int											clientFd = currentClient.getSocket();
	std::string									quitRPL;
	std::string 								quitMessage;
	std::string									quitMessageReason;
	std::vector<std::string>					channelList;

	//SEND QUIT RPL
	for (size_t i = 1; i < requestArguments.size(); i++)
	{
		if (i != requestArguments.size() - 1)
			quitMessageReason += requestArguments[i] + " ";
		else
			quitMessageReason += requestArguments[i];
	}
	quitMessage = "has left the server (reason :" + quitMessageReason + ")";
	quitRPL = ":" + currentClient.getNickname() + "!" + currentClient.getUsername() + "@" + HOSTNAME + " QUIT :" + requestArguments[1] + "\r\n";
	safeSendMessage(clientFd, quitRPL);
	//NOTIFY CHANNELS
	while (it != _Channels.end())
	{
		if (it->second.hasMember(currentClient))
		{
			//CHANNELS WHERE USER IS ALONE 
			if (_Channels[it->first].getMembersList().size() == 1)
				channelList.push_back(it->first);
			else
				it->second.sendMessageToUsers(quitMessage, currentClient.getNickname());
		}
		it++;
	}
	//DISCONNECT USER
	disconnectUserFromServer(clientFd);
	//DELETE CHANNELS WHERE USER WAS THE ONLY MEMBER
	for (size_t i = 0; i < channelList.size(); i++)
	{
		std::string noticeMessage = "NOTICE broadcast :" + channelList[i] + " has been removed for inactivity" + "\r\n";
		_Channels.erase(channelList[i]);
		_ConnectedUsers.broadcastMessage(const_cast<char *>(noticeMessage.c_str()));
	}
	return ;
}

void	IrcServer::pass(std::vector<std::string> &requestArguments, User &currentClient)
{
	if (requestArguments[0] == "PASS" && !currentClient.hasPassword())
	{
		if (requestArguments[1] == _serverPassword)
			currentClient.setHasPassword(true);
		else
		{
			safeSendMessage(currentClient.getSocket(), ERR_PASSWDMISMATCH(currentClient.getNickname()));
			return ;
		}
	}
	else
		safeSendMessage(currentClient.getSocket(), ERR_PASSACCEPTED(currentClient.getNickname()));
	return ;
}

void	IrcServer::nick(std::vector<std::string> &requestArguments, User &currentClient)
{
	if (requestArguments[0] == "NICK" && currentClient.hasPassword())
	{
		User *findClient = _ConnectedUsers.getUser(requestArguments[1]);
		if (findClient == NULL && currentClient.getNickname() == "")
		{
			//NEW CONNECTION
			int nbOfUsers;
			int nbOfChannels;

			_ConnectedUsers.linkUserToNickname(requestArguments[1], currentClient.getSocket());
			currentClient.setNickname(requestArguments[1]);
			nbOfUsers = _ConnectedUsers.size();
			nbOfChannels = _Channels.size();

			std::stringstream statsStream;
			statsStream << ":localhost Current Local Users: " << nbOfUsers << "\r\n"
						<< ":localhost Available Channels: " << nbOfChannels << "\r\n";

			std::string stats = statsStream.str();
			std::string RPLResponse = RPL_WELCOME(requestArguments[1]) +
									RPL_YOURHOST(requestArguments[1]) +
									RPL_CREATED(requestArguments[1], _serverCreationDate) +
									RPL_MYINFO(requestArguments[1]) +
									stats;
			
			safeSendMessage(currentClient.getSocket(), RPLResponse);
		}
		else
		{
			//RENAME REQUEST (ALREADY HAS A NICK)
			if (currentClient.getNickname() != "" && currentClient.getNickname() != requestArguments[1])
			{
				std::string oldNick = currentClient.getNickname();
				User *updatedUser = _ConnectedUsers.updateUser(oldNick, requestArguments[1]);
				if (updatedUser == NULL)
					return ;
				updateMemberInChannels(oldNick, *updatedUser);
				std::string RPLmessage = ":" + oldNick + " NICK " + requestArguments[1] + "\r\n";
				safeSendMessage(currentClient.getSocket(), RPLmessage);
				return ;
			}
			//NEW CONNECTION
			//NICKNAME TAKEN
			if (findClient != NULL && currentClient.getNickname() == "" && findClient->getSocket() != currentClient.getSocket())
			{
				int			nbOfUsers;
				int			nbOfChannels;
				std::string	newNickname = requestArguments[1];

				while (_ConnectedUsers.getUser(newNickname) != NULL)
					newNickname += "_";
				_ConnectedUsers.linkUserToNickname(newNickname, currentClient.getSocket());
				currentClient.setNickname(newNickname);
				nbOfUsers = _ConnectedUsers.size();
				nbOfChannels = _Channels.size();
				
				std::stringstream statsStream;
				statsStream << ":localhost Current Local Users: " << nbOfUsers << "\r\n"
							<< ":localhost Available Channels: " << nbOfChannels << "\r\n";

				std::string stats = statsStream.str();
				std::string RPLResponse =	RPL_WELCOME(newNickname) + \
											RPL_YOURHOST(newNickname) + \
											RPL_CREATED(newNickname, _serverCreationDate) + \
											RPL_MYINFO(newNickname) + \
											stats;
				safeSendMessage(currentClient.getSocket(), RPLResponse);
				return ;
			}
		}
	}
	else
		safeSendMessage(currentClient.getSocket(), ERR_NOTREGISTERED(currentClient.getNickname()));
	return ;
}

void	IrcServer::user(std::vector<std::string> &requestArguments, User &currentClient)
{
	if (requestArguments[0] == "USER" && currentClient.isAuthentificated())
	{
		if (requestArguments.size() < 5)
		{
			safeSendMessage(currentClient.getSocket(), ERR_NEEDMOREPARAMS(currentClient.getNickname(), "USER"));
			return;
		}
		std::string username = requestArguments[1];
		std::string hostname = requestArguments[2];
		std::string servername = requestArguments[3];
		std::string realname = requestArguments[4];
		currentClient.setUserInfo(username, hostname, servername, realname);
	}
	else
		safeSendMessage(currentClient.getSocket(), ERR_NOTREGISTERED(currentClient.getNickname()));
	return ;
}

void	IrcServer::pong(std::vector<std::string> &requestArguments, User &currentClient)
{
	safeSendMessage(currentClient.getSocket(), RPL_PONG(requestArguments[1]));
}


void	IrcServer::dsy_cbarbit_AuthAndChannelMethodsPrototype(int clientFd, std::vector<std::string> requestArguments)
{
	std::string					argument;
	User						*currentClient;

	currentClient = _ConnectedUsers.getUser(clientFd);
	if (!currentClient)
		return ;
	//RETURN IF PASS ISNT VALIDATED YET
	if ((requestArguments[0] != "PASS" && !currentClient->hasPassword()) && requestArguments[0] != "CAP")
	{
		safeSendMessage(currentClient->getSocket(), ERR_NOTREGISTERED(currentClient->getNickname()));
		return ;
	}
	//RETURN IF INVALID ARG NUMBER
	if (requestArguments.size() < 2 && (requestArguments[0] != "LIST"))
		return ;
	
	//HANDLE COMMANDS
	// for (size_t i = 0; i < requestArguments.size(); i++)
	// {
	// 	std::cout << "Handling command : [" << i << "] = [" << requestArguments[i] << "]" << std::endl;
	// }

	if (requestArguments[0] == "PASS")
		pass(requestArguments, *currentClient);
	else if (requestArguments[0] == "NICK")
		nick(requestArguments, *currentClient);
	else if (requestArguments[0] == "USER")
		user(requestArguments, *currentClient);
	else if (requestArguments[0] == "JOIN" && requestArguments.size() > 1 && requestArguments.size() < 4)
		join(requestArguments, *currentClient);
	else if (requestArguments[0] == "PART")
		part(requestArguments, *currentClient);
	else if (requestArguments[0] == "LIST")
		list(requestArguments, *currentClient);
	else if (requestArguments[0] == "PRIVMSG" && requestArguments.size() > 2)
		privmsg(requestArguments, *currentClient);
	else if (requestArguments[0] == "NOTICE" && requestArguments.size() > 2)
		notice(requestArguments, *currentClient);
	else if (requestArguments[0] == "MODE" && requestArguments.size() > 2)
		mode(requestArguments, *currentClient);
	else if (requestArguments[0] == "KICK" && requestArguments.size() > 2)
		kick(requestArguments, *currentClient);
	else if (requestArguments[0] == "INVITE" && requestArguments.size() == 3)
		invite(requestArguments, *currentClient);
	else if (requestArguments[0] == "TOPIC" && requestArguments.size() > 1)
		topic(requestArguments, *currentClient);
	else if (requestArguments[0] == "PING")
		pong(requestArguments, *currentClient);
	else if (requestArguments[0] == "QUIT")
		quit(requestArguments, *currentClient);
	else if (requestArguments[0] == "CAP" && requestArguments[1] == "LS")
		capls(requestArguments, *currentClient);
	else
		safeSendMessage(currentClient->getSocket(), ERR_UNKNOWNCOMMAND(currentClient->getNickname(), requestArguments[0]));
	return ;
}
