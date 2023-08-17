#include "IrcServer.hpp"

void	IrcServer::pass(std::vector<std::string> &requestArguments, User &currentClient)
{
	if (requestArguments[0] == "PASS" && !currentClient.hasPassword())
	{
		if (requestArguments[1] == _serverPassword)
			currentClient.setHasPassword(true);
		else
		{
			safeSendMessage(currentClient.getSocket(), const_cast<char*>(ERR_PASSWDMISMATCH(currentClient.getNickname()).c_str()));
			return ;
		}
	}
	else
		safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_PASSACCEPTED(currentClient.getNickname()).c_str()));
	return ;
}

void	IrcServer::handleCAPLS(int clientFd)
{
	std::vector<std::string>	request;
	std::stringstream			requestSplit;
	std::string					word;
	char						buf[MESSAGE_BUFFER_SIZE];

	recv(clientFd, buf, MESSAGE_BUFFER_SIZE, 0);
	request = splitStringByCRLF(std::string(buf));
	requestSplit << request[0];
	requestSplit >> word;
	if (word == "CAP")
	{
		std::string CAPLS = "CAP * LS :PASS NICK JOIN PRIVMSG NOTICE KICK INVITE MODE PONG\r\n";
		safeSendMessage(clientFd, const_cast<char *>(CAPLS.c_str()));	
	}
	else
		sendWelcomeMessage(clientFd);
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
			
			safeSendMessage(currentClient.getSocket(), const_cast<char*>(RPLResponse.c_str()));
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
				safeSendMessage(currentClient.getSocket(), const_cast<char*>(RPLmessage.c_str()));
				return ;
			}
			//NEW CONNECTION
			//NICKNAME TAKEN
			if (findClient != NULL && currentClient.getNickname() == "" && findClient->getSocket() != currentClient.getSocket())
			{
				int			nbOfUsers;
				int			nbOfChannels;
				std::string	newNickname = requestArguments[1];

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
				safeSendMessage(currentClient.getSocket(), const_cast<char*>(RPLResponse.c_str()));
				return ;
			}
		}
	}
	else
		safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOTREGISTERED(currentClient.getNickname()).c_str()));
	return ;
}

void	IrcServer::user(std::vector<std::string> &requestArguments, User &currentClient)
{
	if (requestArguments[0] == "USER" && currentClient.isAuthentificated())
	{
		if (requestArguments.size() < 5)
		{
			safeSendMessage(currentClient.getSocket(), const_cast<char*>(ERR_NEEDMOREPARAMS(currentClient.getNickname(), "USER").c_str()));
			return;
		}
		std::string username = requestArguments[1];
		std::string hostname = requestArguments[2];
		std::string servername = requestArguments[3];
		std::string realname = requestArguments[4];
		currentClient.setUserInfo(username, hostname, servername, realname);
	}
	else
		safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOTREGISTERED(currentClient.getNickname()).c_str()));
	return ;
}

void	IrcServer::pong(std::vector<std::string> &requestArguments, User &currentClient)
{
	safeSendMessage(currentClient.getSocket(), const_cast<char*>(RPL_PONG(requestArguments[1]).c_str()));
}


void	IrcServer::dsy_cbarbit_AuthAndChannelMethodsPrototype(int clientFd, std::vector<std::string> requestArguments)
{
	std::string					argument;
	User						*currentClient;

	currentClient = _ConnectedUsers.getUser(clientFd);
	//PUT ALL ARGUMENTS IN requestArguments VECTOR
	//RETURN IF INVALID ARG NUMBER
	if (requestArguments.size() < 2)
		return ;
	//RETURN IF PASS ISNT VALIDATED YET
	if (requestArguments[0] != "PASS" && !currentClient->hasPassword())
	{
		safeSendMessage(currentClient->getSocket(), const_cast<char *>(ERR_NOTREGISTERED(currentClient->getNickname()).c_str()));
		return ;
	}
	//HANDLE COMMANDS
	for (size_t i = 0; i < requestArguments.size(); i++)
	{
		std::cout << "Handling command : [" << i << "] = [" << requestArguments[i] << "]" << std::endl;
	}
	if (requestArguments[0] == "PASS")
		pass(requestArguments, *currentClient);
	else if (requestArguments[0] == "NICK")
		nick(requestArguments, *currentClient);
	else if (requestArguments[0] == "USER")
		user(requestArguments, *currentClient);
	else if (requestArguments[0] == "JOIN" && requestArguments.size() > 1)
		join(requestArguments, *currentClient);
	else if (requestArguments[0] == "PART")
		part(requestArguments, *currentClient);
	else if (requestArguments[0] == "WHO")
		who(requestArguments, *currentClient);
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
	return ;
}
