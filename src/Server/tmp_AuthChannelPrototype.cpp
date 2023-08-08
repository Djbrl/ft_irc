#include "IrcServer.hpp"


void	IrcServer::pass(std::vector<std::string> &requestArguments, User &currentClient)
{
	if (requestArguments[0] == "PASS" && !currentClient.hasPassword())
	{
		if (requestArguments[1] == _serverPassword)
		{
			currentClient.setHasPassword(true);
			std::string message = "Set your NICK <nickname> to start using the server.\r\n";
			safeSendMessage(currentClient.getSocket(), const_cast<char*>(message.c_str()));
		}
		else
		{
			std::string message = "Wrong password.\r\n";
			safeSendMessage(currentClient.getSocket(), const_cast<char*>(message.c_str()));
			return ;
		}
	}
	else 	//IF the client is already logged in
	{
		if (requestArguments[0] == "PASS" && currentClient.hasPassword())
		{
			std::string message = "You're already logged in.\r\n";
			safeSendMessage(currentClient.getSocket(), const_cast<char*>(message.c_str()));
			return ;
		}
	}
}

void	IrcServer::nick(std::vector<std::string> &requestArguments, User &currentClient)
{
	if (requestArguments[0] == "NICK" && currentClient.hasPassword())
	{
		//Check if the user is known
		User *isKnownUser = _ConnectedUsers.getUser(requestArguments[1]);
		//IF if its a known user, check if it is THIS client or another client
		if (isKnownUser)
		{
			if (isKnownUser != &currentClient)
			{
				std::string message = "Sorry! This nickname is already taken.\r\n";
				safeSendMessage(currentClient.getSocket(), const_cast<char*>(message.c_str()));
				return ;
			}
			else
			{
				std::string message = "Your nickname has already been set.\r\n";
				safeSendMessage(currentClient.getSocket(), const_cast<char*>(message.c_str()));
				return ;
			}
		}
		else		//IF the user is not known, check if it already has a nickname (update), otherwise set it
		{
			if (&currentClient == _ConnectedUsers.getUser(currentClient.getSocket()) && currentClient.getNickname() != "")
			{
				this->_ConnectedUsers.linkUserToNickname(requestArguments[1], currentClient.getSocket());
				currentClient.setNickname(requestArguments[1]);
				std::string message = "Your username has been updated to [" + currentClient.getNickname() + "].\r\n";
				safeSendMessage(currentClient.getSocket(), const_cast<char*>(message.c_str()));
				return ;
			}
			else
			{
				this->_ConnectedUsers.linkUserToNickname(requestArguments[1], currentClient.getSocket());
				currentClient.setNickname(requestArguments[1]);
				std::string message = "Hello " + currentClient.getNickname() + "! You now have user-access to the server.\r\n";
				safeSendMessage(currentClient.getSocket(), const_cast<char*>(message.c_str()));
				std::string numeric001 = "001 " + currentClient.getNickname() + " :Welcome to the IRC network, " + currentClient.getNickname() + "!\r\n";
				safeSendMessage(currentClient.getSocket(), const_cast<char *>(numeric001.c_str()));
			}
		}
	}
}

void IrcServer::join(std::vector<std::string> &requestArguments, User &currentClient)
{
	if (requestArguments[0] == "JOIN" && currentClient.isAuthentificated())
	{
		if (requestArguments[1].size() > 1 && requestArguments[1].substr(0, 1) == "#")
		{
			std::map<std::string, Channel>::iterator	isExistingChannel;
			std::string									channelName = requestArguments[1];
			std::string									message = "[" + channelName + "]\r\n";
			
			isExistingChannel = _Channels.find(channelName);
			if (isExistingChannel == _Channels.end())
				addChannel(channelName, currentClient);
			else
			{
				if (_Channels[channelName].hasMember(currentClient))
				{
					std::string message = "You're already in " + channelName + " !\r\n";
					safeSendMessage(currentClient.getSocket(), const_cast<char *>(message.c_str()));				
					return ;
				}
				else
				{
					_Channels[channelName].addMember(currentClient);
					_Channels[channelName].showMessageHistory(currentClient);
				}
			}
			safeSendMessage(currentClient.getSocket(), const_cast<char *>(message.c_str()));
			//uncomment to debug channel
			// std::cout << _Channels[channelName] << std::endl;
		}
		else
		{
			std::string message = "Invalid channel name.\r\n";
			safeSendMessage(currentClient.getSocket(), const_cast<char *>(message.c_str()));
			return ;
		}
	}
}

void	IrcServer::privmsg(std::vector<std::string> &requestArguments, User &currentClient)
{
	if (requestArguments[0] == "PRIVMSG" && currentClient.isAuthentificated())
	{
		std::map<std::string, Channel>::iterator	isExistingChannel;
		std::string									messageToChannel;
		std::string									channelName = requestArguments[1];

		//BUILD MESSAGE BY MERGING ARGUMENTS
		for (int i = 2; i < (int)requestArguments.size(); i++)
			messageToChannel += requestArguments[i] + " ";
		//IF VALID CHANNEL NAME
		if (requestArguments[1].size() > 1 && requestArguments[1].substr(0, 1) == "#")
		{
			isExistingChannel = _Channels.find(channelName);
			if (isExistingChannel == _Channels.end())
			{
				std::string message = "Non-existent channel : "+ channelName +"\r\n";
				safeSendMessage(currentClient.getSocket(), const_cast<char*>(message.c_str()));
				return ;
			}
			_Channels[channelName].sendMessageToUsers(messageToChannel, currentClient.getNickname());
		}
		else
		{
			std::string message = "Invalid PRIVMSG args.\r\n";
			safeSendMessage(currentClient.getSocket(), const_cast<char*>(message.c_str()));
			return ;
		}
	}
}

void	IrcServer::pong(std::vector<std::string> &requestArguments, User &currentClient)
{
	//replace localhost with serverAddr 
    std::string pongReply = "PONG localhost :" + requestArguments[1];
    safeSendMessage(currentClient.getSocket(), const_cast<char*>(pongReply.c_str()));
}


void	IrcServer::dsy_cbarbit_AuthAndChannelMethodsPrototype(int clientFd, char *socketData)
{
	std::vector<std::string> 	requestArguments;
	std::stringstream			request(socketData);
	std::string					argument;
	User						*currentClient;

	currentClient = _ConnectedUsers.getUser(clientFd);
	//PUT ALL ARGUMENTS IN requestArguments VECTOR
	while (request >> argument)
		requestArguments.push_back(argument);
	//RETURN IF INVALID ARG NUMBER
	if (requestArguments.size() < 2)
		return ;
	//RETURN IF PASS ISNT VALIDATED YET
	if (requestArguments[0] != "PASS" && currentClient->hasPassword() == false)
	{
		std::string message = "Please enter the server password first.\r\n";
		safeSendMessage(clientFd, const_cast<char*>(message.c_str()));
		return ;
	}
	//HANDLE COMMANDS
	if (requestArguments[0] == "PASS")
		pass(requestArguments, *currentClient);
	else if (requestArguments[0] == "NICK")
		nick(requestArguments, *currentClient);
	else if (requestArguments[0] == "JOIN")
		join(requestArguments, *currentClient);
	else if (requestArguments[0] == "PRIVMSG" && requestArguments.size() > 2)
		privmsg(requestArguments, *currentClient);
	else if (requestArguments[0] == "PING")
		pong(requestArguments, *currentClient);
		return ;
	//KICK argument PROTOTYPE
	// if (argument == "KICK")
	// {
	//     std::map<std::string, Channel>::iterator    it;
	//     int                                         j;

	//     if ((it = this->isAChannel(first arg) != this->_Channels.end()))
	//     {
	//         if (it->isChannelOp(*user)) //user is operator (first arg)
	//         {
	//             (if (j = it->isAMember(second arg)) != -1)  //user is in the channel (second arg)
	//             {
	//                 it->removeMember(it->_membersList[i]);
	//             }
	//             else
	//             {
	//                 std::string message = "Sorry, the member you want to remove is not in the channel.\r\n";
	//                 safeSendMessage(clientFd, const_cast<char*>(message.c_str()));
	//                 return ;                      
	//             }
	//         }
	//         else
	//         {
	//             std::string message = "Sorry, you are not the channel operator.\r\n";
	//             safeSendMessage(clientFd, const_cast<char*>(message.c_str()));
	//             return ;                
	//         }
	//     }
	// }
}