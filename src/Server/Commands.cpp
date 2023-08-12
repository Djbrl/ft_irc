#include "IrcServer.hpp"

void IrcServer::join(std::vector<std::string> &requestArguments, User &currentClient)
{
	if (requestArguments[0] == "JOIN" && currentClient.isAuthentificated())
	{
		bool isValidChannelName = requestArguments[1].size() > 1 && requestArguments[1][0] == '#';
		if (isValidChannelName)
		{
			std::map<std::string, Channel>::iterator	isExistingChannel;
			std::string									channelName = requestArguments[1];

			//NEW CHANNEL
			isExistingChannel = _Channels.find(channelName);
			if (isExistingChannel == _Channels.end())
			{
				addChannel(channelName, currentClient);
				std::string RPLResponse =	RPL_TOPIC(currentClient.getNickname(), channelName, _Channels[channelName].getChannelTopic())		+ \
											RPL_NAMREPLY(currentClient.getNickname(), channelName, _Channels[channelName].printMemberList())	+ \
											RPL_ENDOFNAMES(currentClient.getNickname(), channelName);
				safeSendMessage(currentClient.getSocket(), const_cast<char *>(RPLResponse.c_str()));
			}
			//EXISTING CHANNEL
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

void IrcServer::part(std::vector<std::string> &requestArguments, User &currentClient)
{
	if (requestArguments[0] == "PART" && currentClient.isAuthentificated())
	{
		std::string	channelName = requestArguments[1];
		bool		isValidChannelName = channelName.size() > 1 && channelName[0] == '#';
		
		if (isValidChannelName || _Channels.find(channelName) != _Channels.end())
		{
			if (_Channels[channelName].hasMember(currentClient))
			{
				//REMOVE MEMBER
				_Channels[channelName].removeMember(currentClient);
				if (_Channels[channelName].getChannelOwner() == currentClient)
				{
					_Channels[channelName].sendMessageToUsers("(owner) has left the channel", currentClient.getNickname());
					safeSendMessage(currentClient.getSocket(), const_cast<char *>(RPL_PARTNOTICE(currentClient.getNickname(), channelName).c_str()));		
				}
				else
				{
					_Channels[channelName].sendMessageToUsers("left the channel", currentClient.getNickname());
					safeSendMessage(currentClient.getSocket(), const_cast<char *>(RPL_PARTNOTICE(currentClient.getNickname(), channelName).c_str()));
				}
				//EMPTY CHANNEL
				if (_Channels[channelName].getMembersList().size() == 0)
				{
					_Channels.erase(channelName);
					std::string noticeMessage = "NOTICE " + requestArguments[1] + " :" + channelName + " has been removed for inactivity" + "\r\n";
					safeSendMessage(currentClient.getSocket(), const_cast<char *>(noticeMessage.c_str()));
				}
			}
			else
				safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_USERNOTINCHANNEL(currentClient.getNickname(), channelName).c_str()));
		}
		else
			safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOSUCHCHANNEL(currentClient.getNickname(), requestArguments[1]).c_str()));
	}
	else
		safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOTREGISTERED(currentClient.getNickname()).c_str()));
	return ;
}

void IrcServer::who(std::vector<std::string> &requestArguments, User &currentClient)
{
	if (requestArguments[0] == "WHO" && currentClient.isAuthentificated())
	{
		//UNKNOWN CHANNEL
		if (_Channels.find(requestArguments[1]) == _Channels.end())
		{
			safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOSUCHCHANNEL(currentClient.getNickname(), requestArguments[1]).c_str()));
			return ;
		}

		//SEND RESPONSE TO IRSSI
		std::string	channelName = requestArguments[1];
		Channel 	channel = _Channels[requestArguments[1]];
		if (channelName.size() > 1 && channelName[0] == '#')
		{
			std::string userList = channel.printMemberList();
			std::string whoIsResponse = ":ft_irc 352 " + currentClient.getNickname() + " " + channelName + " " + \
										"ft_irc" + " " + channelName + " " + \
										currentClient.getNickname() + " H :* " + currentClient.getRealname() + "\r\n";
			std::string endWhoIsResponse =	":ft_irc 315 " + currentClient.getNickname() + " " + channelName + \
											" :End of WHO list\r\n";
			std::string	RPLResponse = whoIsResponse + endWhoIsResponse;
			safeSendMessage(currentClient.getSocket(), const_cast<char *>(RPLResponse.c_str()));
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
		
		bool isChannelName = requestArguments[1].size() > 1 && requestArguments[1][0] == '#';
		//SEND TO CHANNEL
		if (isChannelName)
		{
			isExistingChannel = _Channels.find(channelName);
			if (isExistingChannel != _Channels.end())
				_Channels[channelName].sendMessageToUsers(messageToChannel, currentClient.getNickname());
			else
				safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOSUCHCHANNEL(currentClient.getNickname(), channelName).c_str()));
		}
		//SEND TO USER
		else
		{
			if (_ConnectedUsers.getUser(requestArguments[1]) != NULL)
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
	return ;
}

void	IrcServer::notice(std::vector<std::string> &requestArguments, User &currentClient)
{
	if (requestArguments[0] == "NOTICE" && currentClient.isAuthentificated())
	{
		std::map<std::string, Channel>::iterator	isExistingChannel;
		std::string									messageToChannel = "NOTICE ";
		std::string									channelName = requestArguments[1];

		//BUILD MESSAGE BY MERGING ARGUMENTS
		for (int i = 2; i < (int)requestArguments.size(); i++)
			messageToChannel += requestArguments[i] + " ";
		
		bool isChannelName = requestArguments[1].size() > 1 && requestArguments[1][0] == '#';
		if (isChannelName)
		{
			isExistingChannel = _Channels.find(channelName);
			if (isExistingChannel != _Channels.end())
				_Channels[channelName].sendNoticeToUsers(messageToChannel, currentClient.getNickname());
			else
				safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOSUCHCHANNEL(currentClient.getNickname(), channelName).c_str()));
		}
		else
		{
			if (_ConnectedUsers.getUser(requestArguments[1]) != NULL)
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
	return ;
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
