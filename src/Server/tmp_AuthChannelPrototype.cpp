#include "IrcServer.hpp"


void IrcServer::pass(std::vector<std::string> &requestArguments, User &currentClient)
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

void IrcServer::join(std::vector<std::string> &requestArguments, User &currentClient)
{
	if (requestArguments[0] == "JOIN" && currentClient.isAuthentificated())
	{
		bool isValidChannelName = requestArguments[1].size() > 1 && requestArguments[1].substr(0, 1) == "#";
		if (isValidChannelName)
		{
			std::map<std::string, Channel>::iterator	isExistingChannel;
			std::string									channelName(requestArguments[1]);

			isExistingChannel = _Channels.find(channelName);
			if (isExistingChannel == _Channels.end())
			{
				addChannel(channelName, currentClient);
				std::string RPLResponse =	RPL_TOPIC(currentClient.getNickname(), channelName, _Channels[channelName].getChannelTopic())		+ \
											RPL_NAMREPLY(currentClient.getNickname(), channelName, _Channels[channelName].printMemberList())	+ \
											RPL_ENDOFNAMES(currentClient.getNickname(), channelName);
				safeSendMessage(currentClient.getSocket(), const_cast<char *>(RPLResponse.c_str()));
			}
			else
			{
				if (_Channels[channelName].hasMember(currentClient))
					safeSendMessage(currentClient.getSocket(), const_cast<char *>(RPL_ALREADYREGISTRED(currentClient.getNickname(), requestArguments[1]).c_str()));
				else
				{
					std::string notification = "joined the channel";
					_Channels[channelName].sendMessageToUsers(notification, currentClient.getNickname());
					_Channels[channelName].addMember(currentClient);
					std::string RPLResponse =	RPL_TOPIC(currentClient.getNickname(), channelName, _Channels[channelName].getChannelTopic())		+ \
												RPL_NAMREPLY(currentClient.getNickname(), channelName, _Channels[channelName].printMemberList())	+ \
												RPL_ENDOFNAMES(currentClient.getNickname(), channelName);
					std::cout << _Channels[channelName] << std::endl;
					safeSendMessage(currentClient.getSocket(), const_cast<char *>(RPLResponse.c_str()));
					usleep(50000);
					_Channels[channelName].showMessageHistory(currentClient);
				}
			}
		}
		else
			safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOSUCHCHANNEL(currentClient.getNickname(), requestArguments[1]).c_str()));
	}
	else
		safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOTREGISTERED(currentClient.getNickname()).c_str()));
	return ;
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
		bool isChannelName = requestArguments[1].size() > 1 && requestArguments[1].substr(0, 1) == "#";
		if (isChannelName)
		{
			isExistingChannel = _Channels.find(channelName);
			if (isExistingChannel == _Channels.end())
				safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOSUCHCHANNEL(currentClient.getNickname(), channelName).c_str()));
			else
				_Channels[channelName].sendMessageToUsers(messageToChannel, currentClient.getNickname());
		}
		else
		{
			if (_ConnectedUsers[requestArguments[1]].isAuthentificated())
			{
				std::string DM = ":" + currentClient.getNickname() + " PRIVMSG " + _ConnectedUsers[requestArguments[1]].getNickname() + " :" + messageToChannel + "\r\n";
				safeSendMessage(_ConnectedUsers[requestArguments[1]].getSocket(), const_cast<char *>(DM.c_str()));
			}
			else
				safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOSUCHNICK(currentClient.getNickname(), requestArguments[1]).c_str()));
		}
	}
	else
		safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOTREGISTERED(currentClient.getNickname()).c_str()));
}

void	IrcServer::notice(std::vector<std::string> &requestArguments, User &currentClient)
{
	if (requestArguments[0] == "NOTICE" && currentClient.isAuthentificated())
	{
		std::map<std::string, Channel>::iterator	isExistingChannel;
		std::string									messageToChannel;
		std::string									channelName = requestArguments[1];

		//BUILD MESSAGE BY MERGING ARGUMENTS
		for (int i = 2; i < (int)requestArguments.size(); i++)
			messageToChannel += requestArguments[i] + " ";
		//IF VALID CHANNEL NAME
		bool isChannelName = requestArguments[1].size() > 1 && requestArguments[1].substr(0, 1) == "#";
		if (isChannelName)
		{
			isExistingChannel = _Channels.find(channelName);
			if (isExistingChannel == _Channels.end())
				safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOSUCHCHANNEL(currentClient.getNickname(), channelName).c_str()));
			else
			{
				if (_Channels[channelName].hasMember(currentClient.getNickname()))
					_Channels[channelName].sendNoticeToUsers(messageToChannel, currentClient.getNickname());
			}
		}
		else
		{
			if (_ConnectedUsers[requestArguments[1]].isAuthentificated())
			{
				std::string noticeMessage = "NOTICE " + requestArguments[1] + " :" + messageToChannel + "\r\n";
				safeSendMessage(_ConnectedUsers[requestArguments[1]].getSocket(), const_cast<char *>(noticeMessage.c_str()));
			}
			else
				safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOSUCHNICK(currentClient.getNickname(), requestArguments[1]).c_str()));
		}
	}
	else
		safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOTREGISTERED(currentClient.getNickname()).c_str()));
}


void	IrcServer::kick(std::vector<std::string> &requestArguments, User &currentClient)
{
	//No need to verify if requestArguments[0] == "KICK" -> it is done below (HANDLE COMMANDS)
	if (currentClient.isAuthentificated())
	{
		std::map<std::string, Channel>::iterator	isExistingChannel;
		std::vector<User>::iterator					isExistingUser;
		std::string									channelName = requestArguments[1];
		std::string									userToRemove = requestArguments[2];
		
		//check if channel exists
		isExistingChannel = _Channels.find(channelName);
		if (isExistingChannel == _Channels.end())
		{
			safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOSUCHCHANNEL(currentClient.getNickname(), channelName).c_str()));
			return ; 
		}
		else
		{
			//check if the user to remove is in the channel
			isExistingUser = isExistingChannel->second.isAMember(userToRemove);
			if (isExistingUser == isExistingChannel->second.getMembersList().end())
			{
				std::string message = "Sorry, the user you want to remove is not in the channel.\r\n";
				safeSendMessage(currentClient.getSocket(), const_cast<char*>(message.c_str()));
				return ;				
			}
			else
			{
				//check if the current user is channel operator
				if (!isExistingChannel->second.isChannelOp(currentClient))
				{
					safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_CHANOPRIVSNEEDED(currentClient.getNickname(), channelName).c_str()));
					return ;
				}
				else
				{
					isExistingChannel->second.removeMember(*isExistingUser);
					std::string	messageToKicked;
					std::string messageToKicker = "User has successfully been removed !\r\n";
					safeSendMessage(currentClient.getSocket(), const_cast<char*>(messageToKicker.c_str()));
					
					//send an explanation to the user who has been kicked out
					if (requestArguments.size() > 3)
					{
						messageToKicked = "Reason for being kicked, according to Channel Operator ";
						size_t	found;
						if ((found = requestArguments[3].find(":")) == std::string::npos)
							messageToKicked += ":";
						for (int i = 3; i < (int)requestArguments.size(); i++)
							messageToKicked += requestArguments[i] + " ";
						messageToKicked += ".\r\n";
						safeSendMessage(isExistingUser->getSocket(), const_cast<char*>(messageToKicked.c_str()));
					}
					else
					{
						messageToKicked = "You have been kicked of channel " + requestArguments[1] + " by channel operator.\r\n";
						safeSendMessage(isExistingUser->getSocket(), const_cast<char*>(messageToKicked.c_str()));
					}
					return ;
				}
			}
		}
	}
	else
		safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOTREGISTERED(currentClient.getNickname()).c_str()));
}

void	IrcServer::invite(std::vector<std::string> &requestArguments, User &currentClient)
{		
	
	//NOTES MAY NEED LATER
		// 	isMember = isExistingChannel->second.hasMember(currentClient); //check if the currenClient is in the channel
		// 	if (!isMember)
		// 	{
		// 		std::string message = "Sorry, you are not member of the channel and therefore cannot invite anyone.\r\n";
		// 		safeSendMessage(currentClient.getSocket(), const_cast<char*>(message.c_str()));
		// 		return ;
		// 	}



	//No need to verify if requestArguments[0] == "INVITE" -> it is done below (HANDLE COMMANDS)
	if (currentClient.isAuthentificated())
	{
		std::map<std::string, Channel>::iterator	isExistingChannel;
		std::vector<User>::iterator					isUserToInvite;
		std::string									userToInvite = requestArguments[1];
		std::string									channelName = requestArguments[2];

		//check if userToInvite exists
		if (!_ConnectedUsers.userExists(userToInvite))
		{
			safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOSUCHNICK(currentClient.getNickname(), userToInvite).c_str()));
			return ;
		}
		else
		{
			isExistingChannel = _Channels.find(channelName); //check if the channel exists just to check if user already is in it !
			if (isExistingChannel != _Channels.end())
			{
				//check if userToInvite already on channel
				isUserToInvite = isExistingChannel->second.isAMember(userToInvite);
				if (isUserToInvite != isExistingChannel->second.getMembersList().end())
				{
					safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_USERONCHANNEL(currentClient.getNickname(), userToInvite, channelName).c_str()));
					return ;
				}
			}
			else
			{
				//STILL NEED TO DO : before removing, check if it is an invite-only channel - if yes, then currenClientmust be a Channel Operator
				safeSendMessage(currentClient.getSocket(), const_cast<char *>(RPL_INVITING(currentClient.getNickname(), userToInvite, channelName).c_str()));

				isUserToInvite->setChannelsList(channelName); //add this channel to the channel list the user has been invited to
				std::string	messageToInvitedUser = currentClient.getNickname() + " invited you to join channel " + requestArguments[2] + ".\r\n";
				safeSendMessage(_ConnectedUsers.getUser(userToInvite)->getSocket(), const_cast<char*>(messageToInvitedUser.c_str()));
				return ;
			}
		}
	}
	else
		safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOTREGISTERED(currentClient.getNickname()).c_str()));
}

void	IrcServer::topic(std::vector<std::string> &requestArguments, User &currentClient)
{

	//No need to verify if requestArguments[0] == "TOPIC" -> it is done below (HANDLE COMMANDS)
	if (currentClient.isAuthentificated())
	{
		std::map<std::string, Channel>::iterator	isExistingChannel;
		std::string									channelName = requestArguments[1];
	
		//check if channel exists
		isExistingChannel = _Channels.find(channelName);
		if (isExistingChannel == _Channels.end())
		{
			std::string message = "Sorry, the channel you entered does not exist.\r\n";
			safeSendMessage(currentClient.getSocket(), const_cast<char*>(message.c_str()));
			return ; 
		}
		else
		{
			//it should just print out the channel's topic if defined
			if (requestArguments.size() == 2)
			{
				std::string	channelTopic = isExistingChannel->second.getChannelTopic();
				if (channelTopic.empty())
				{
					safeSendMessage(currentClient.getSocket(), const_cast<char *>(RPL_NOTOPIC(currentClient.getNickname(), channelName).c_str()));
					return ;
				}
				else
				{
					safeSendMessage(currentClient.getSocket(), const_cast<char *>(RPL_TOPIC(currentClient.getNickname(), channelName, isExistingChannel->second.getChannelTopic()).c_str()));
					return ;
				}
			}
			//it should update the topic, only if the channel mode allows it
			// else if (requestArguments.size() == 3)
			// {

			// }
		}
	}
	else
		safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOTREGISTERED(currentClient.getNickname()).c_str()));

}


void	IrcServer::mode(std::vector<std::string> &requestArguments, User &currentClient)
{

	//No need to verify if requestArguments[0] == "MODE" -> it is done below (HANDLE COMMANDS)
	if (currentClient.isAuthentificated())
	{
		std::map<std::string, Channel>::iterator	isExistingChannel;
		std::string									channelName = requestArguments[1];
		
		//check if channel exists
		isExistingChannel = _Channels.find(channelName);
		if (isExistingChannel == _Channels.end())
		{
			std::string message = "Sorry, the channel you entered does not exist.\r\n";
			safeSendMessage(currentClient.getSocket(), const_cast<char*>(message.c_str()));
			return ; 
		}
		else
		{
			if (!isExistingChannel->second.isChannelOp(currentClient)) //the user is not channel operator
			{
				std::string message = "Sorry, you are not a channel operator. Therefore, you cannot kick a member.\r\n";
				safeSendMessage(currentClient.getSocket(), const_cast<char*>(message.c_str()));
				return ;
			}
			else
			{
				//

			}
		}
	}
	else
		safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOTREGISTERED(currentClient.getNickname()).c_str()));

}


void	IrcServer::pong(std::vector<std::string> &requestArguments, User &currentClient)
{
	safeSendMessage(currentClient.getSocket(), const_cast<char*>(RPL_PONG(requestArguments[1]).c_str()));
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
	if (requestArguments[0] != "PASS" && !currentClient->hasPassword())
	{
		safeSendMessage(currentClient->getSocket(), const_cast<char *>(ERR_NOTREGISTERED(currentClient->getNickname()).c_str()));
		return ;
	}
	//HANDLE COMMANDS
	std::cout << "Handling command : [" << socketData << "]" << std::endl;
	if (requestArguments[0] == "PASS")
		pass(requestArguments, *currentClient);
	else if (requestArguments[0] == "NICK")
		nick(requestArguments, *currentClient);
	else if (requestArguments[0] == "JOIN")
		join(requestArguments, *currentClient);
	else if (requestArguments[0] == "PRIVMSG" && requestArguments.size() > 2)
		privmsg(requestArguments, *currentClient);
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
