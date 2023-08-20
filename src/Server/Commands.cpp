#include "IrcServer.hpp"

std::vector<std::string>	IrcServer::splitJoinArgument(std::string &argument)
{
	std::vector<std::string>	splitArgument;
	std::string					subString;

	std::size_t argumentLen = argument.size(); //size of the argument I receive
	std::size_t start = 0;
	std::size_t end, found;

	found = argument.find(',');
	while (found != std::string::npos && found != (argumentLen - 1))
	{
		end = found;
		subString = argument.substr(start, end - start);
		splitArgument.push_back(subString);
		start = found + 1;
		//find the next occurrence of , in the argument
		found = argument.find(',', start);
	}
	if (found == std::string::npos || found == (argumentLen - 1))
	{
		if (found == argumentLen - 1)
			subString = argument.substr(start, (argumentLen - 1) - start);
		else
			subString = argument.substr(start, argumentLen - start);
		splitArgument.push_back(subString);
	}
	return (splitArgument);
}

std::vector<std::string>	IrcServer::parseChannels(std::vector<std::string> channels, User &currentClient)
{
	std::vector<std::string>	validChannels;

	for (std::size_t i = 0; i < channels.size(); i++)
	{
		//check if channel's name is valid
		if (channels[i].size() < 2 || (channels[i][0] != '#' && channels[i][0] != '&'))
			safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOSUCHCHANNEL(currentClient.getNickname(), channels[i]).c_str()));
		else
			validChannels.push_back(channels[i]);
	}
	return (validChannels);
}


void	IrcServer::createChannel(const std::string &channelName, User &currentClient)
{
	std::cout << "Channel does not exist" << std::endl;
	addChannel(channelName, currentClient);
	std::string RPLResponse =	RPL_TOPIC(currentClient.getNickname(), channelName, _Channels[channelName].getChannelTopic()) + 
								RPL_NAMREPLY(currentClient.getNickname(), channelName, _Channels[channelName].printMemberList()) + 
								RPL_ENDOFNAMES(currentClient.getNickname(), channelName);
	safeSendMessage(currentClient.getSocket(), const_cast<char *>(RPLResponse.c_str()));
}

void	IrcServer::joinChannel(const std::string &channelName, User &currentClient)
{
	std::cout << "Channel already exists" << std::endl;
	if (_Channels[channelName].hasMember(currentClient))
		safeSendMessage(currentClient.getSocket(), const_cast<char *>(RPL_ALREADYREGISTRED(currentClient.getNickname(), channelName).c_str()));
	else
	{
		std::string notification = "joined the channel";
		_Channels[channelName].sendMessageToUsers(notification, currentClient.getNickname());
		_Channels[channelName].addMember(currentClient);
		std::string RPLResponse =	RPL_TOPIC(currentClient.getNickname(), channelName, _Channels[channelName].getChannelTopic())		+ 
									RPL_NAMREPLY(currentClient.getNickname(), channelName, _Channels[channelName].printMemberList())	+ 
									RPL_ENDOFNAMES(currentClient.getNickname(), channelName);
		safeSendMessage(currentClient.getSocket(), const_cast<char *>(RPLResponse.c_str()));
		usleep(50000);
		_Channels[channelName].showMessageHistory(currentClient);
	}
}

int	IrcServer::checkChannelExceptions(std::map<std::string, Channel>::iterator	&channel, std::vector<std::string> passwords, std::size_t channelIndex, User &currentClient)
{
	std::string channelName = channel->second.getChannelName();
	
	//Look for +i in channel's mode
	if (channel->second.findMode("+i"))
	{
		//check if client has been invited !
		if (!currentClient.channelIsInList(channelName))
		{
			safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_INVITEONLYCHAN(channelName).c_str()));
			return (-1); //cannot join this channel
		}
	}
	std::string	password = channel->second.getChannelPassword();
	if (!password.empty())
	{
		//if the user entered a password - check it is the write password
		if (channelIndex < passwords.size())
		{
			if(password != passwords[channelIndex])
			{
				safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_BADCHANNELKEY(channelName).c_str()));
				return (-1);
			}
			else
				return (0);
		}
		else
		{
			safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_BADCHANNELKEY(channelName).c_str()));
			return (-1);
		}
	}
	return (0); //user can join the channel
}

void IrcServer::join(std::vector<std::string> &requestArguments, User &currentClient)
{
	std::vector<std::string>	channels;
	std::vector<std::string>	passwords;

	if (currentClient.isAuthentificated())
	{
		//get all channels
		channels = splitJoinArgument(requestArguments[1]);

		//time to check if channels are valid in themselves
		std::vector<std::string>	validChannels;
		if ((validChannels = parseChannels(channels, currentClient)).empty())
			return ;
		//if validChannels is not empty, this means that at least one channel is valid
		else
		{
			//if password is empty, it means that no password has been provided
			if (requestArguments.size() == 3)
			{
				//get all passwords (if there are any)
				passwords = splitJoinArgument(requestArguments[2]);
			}
			for (std::size_t i = 0; i < validChannels.size(); i++)
			{
				std::string									channelName;
				std::map<std::string, Channel>::iterator	isExistingChannel;

				channelName = validChannels[i];
				isExistingChannel = _Channels.find(channelName);
				
				//option 1: the channel does not exist
				if (isExistingChannel == _Channels.end())
					createChannel(channelName, currentClient);
				//option 2: the channel exists
				else
				{
					int retCheck = checkChannelExceptions(isExistingChannel, passwords, i, currentClient);
					//if all conditions are met, then it is possible to join channel
					if (retCheck == 0)
						joinChannel(channelName, currentClient);
				}
			}

		}

		//TEST channel
			for (std::size_t i = 0; i < validChannels.size(); i++)
				std::cout << "CHAN is : " << validChannels[i] << std::endl;

		//TEST password
			for (std::size_t i = 0; i < passwords.size(); i++)
				std::cout << "PASS is : " << passwords[i] << std::endl;
	}
	else
		safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOTREGISTERED(currentClient.getNickname()).c_str()));
}	
	
// 	if (requestArguments[0] == "JOIN" && currentClient.isAuthentificated())
// 	{
// 		bool isValidChannelName = requestArguments[1].size() > 1 && requestArguments[1][0] == '#';
// 		if (isValidChannelName)
// 		{
// 			std::map<std::string, Channel>::iterator	isExistingChannel;
// 			std::string									channelName = requestArguments[1];

// 			//NEW CHANNEL
// 			isExistingChannel = _Channels.find(channelName);
// 			if (isExistingChannel == _Channels.end())
// 			{
// 				addChannel(channelName, currentClient);
// 				std::string RPLResponse =	RPL_TOPIC(currentClient.getNickname(), channelName, _Channels[channelName].getChannelTopic())		+ 
// 											RPL_NAMREPLY(currentClient.getNickname(), channelName, _Channels[channelName].printMemberList())	+ 
// 											RPL_ENDOFNAMES(currentClient.getNickname(), channelName);
// 				safeSendMessage(currentClient.getSocket(), const_cast<char *>(RPLResponse.c_str()));
// 			}
// 			//EXISTING CHANNEL
// 			else
// 			{
// 				if (_Channels[channelName].hasMember(currentClient))
// 					safeSendMessage(currentClient.getSocket(), const_cast<char *>(RPL_ALREADYREGISTRED(currentClient.getNickname(), requestArguments[1]).c_str()));
// 				else
// 				{
// 					std::string notification = "joined the channel";
// 					_Channels[channelName].sendMessageToUsers(notification, currentClient.getNickname());
// 					_Channels[channelName].addMember(currentClient);
// 					std::string RPLResponse =	RPL_TOPIC(currentClient.getNickname(), channelName, _Channels[channelName].getChannelTopic())		+ 
// 												RPL_NAMREPLY(currentClient.getNickname(), channelName, _Channels[channelName].printMemberList())	+ 
// 												RPL_ENDOFNAMES(currentClient.getNickname(), channelName);
// 					safeSendMessage(currentClient.getSocket(), const_cast<char *>(RPLResponse.c_str()));
// 					usleep(50000);
// 					_Channels[channelName].showMessageHistory(currentClient);
// 				}
// 			}
// 		}
// 		else
// 			safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOSUCHCHANNEL(currentClient.getNickname(), requestArguments[1]).c_str()));
// 	}
// 	else
// 		safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOTREGISTERED(currentClient.getNickname()).c_str()));
// 	return ;
// }

void IrcServer::part(std::vector<std::string> &requestArguments, User &currentClient)
{
	if (requestArguments[0] == "PART" && currentClient.isAuthentificated())
	{
		std::string	channelName = requestArguments[1];
		bool		isValidChannelName = channelName.size() > 1 && (channelName[0] == '#' || channelName[0] == '&');
		
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
		if (channelName.size() > 1 && (channelName[0] == '#' || channelName[0] == '&'))
		{
			std::string userList = channel.printMemberList();
			std::string whoIsResponse = ":ft_irc 352 " + currentClient.getNickname() + " " + channelName + " " + \
										"ft_irc" + " " + channelName + " " + \
										currentClient.getNickname() + " H :* " + currentClient.getRealname() + "\r\n";
			std::string endWhoIsResponse =	":ft_irc 315 " + currentClient.getNickname() + " " + channelName + \
											" :End of WHO\r\n";
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

void IrcServer::list(std::vector<std::string> &requestArguments, User &currentClient)
{
	(void)requestArguments;
    if (currentClient.isAuthentificated())
    {
        std::map<std::string, Channel>::const_iterator it = _Channels.begin();
        while (it != _Channels.end())
        {
			std::vector<std::string>	modesList = it->second.getModesList();
			std::stringstream			nbUsers;
            std::string					userList = it->second.printMemberList();
            std::string					topic = it->second.getChannelTopic();
			std::string					modes = "+";
            Channel						channel = it->second;

			nbUsers << channel.getMembersList().size();
			for (size_t i = 0; i < modesList.size(); i++)
				modes += modesList[i][1];
            std::string listResponse =	":ft_irc 322 " + currentClient.getNickname() + " " + it->first + " " +
                                    	nbUsers.str() + " " + "[" + modes + "]" + " :" + topic + "\r\n";
            safeSendMessage(currentClient.getSocket(), const_cast<char *>(listResponse.c_str()));
			it++;
        }
        std::string endListResponse = ":ft_irc 323 " + currentClient.getNickname() + " :End of /LIST\r\n";
        safeSendMessage(currentClient.getSocket(), const_cast<char *>(endListResponse.c_str()));
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
		
		bool isChannelName = requestArguments[1].size() > 1 && (requestArguments[1][0] == '#' || requestArguments[1][0] == '&');
		//SEND TO CHANNEL
		if (isChannelName)
		{
			isExistingChannel = _Channels.find(channelName);
			if (isExistingChannel != _Channels.end() && isExistingChannel->second.hasMember(currentClient))
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
		
		bool isChannelName = requestArguments[1].size() > 1 && (requestArguments[1][0] == '#' || requestArguments[1][0] == '&');
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
		bool										isMember;

		//check if channel exists
		isExistingChannel = _Channels.find(channelName);
		if (isExistingChannel == _Channels.end())
		{
			safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOSUCHCHANNEL(currentClient.getNickname(), channelName).c_str()));
			return ;
		}
		else
		{
			//check if the user kicking is in channel
			isMember = isExistingChannel->second.hasMember(currentClient);
			if (!isMember)
			{
				safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOTONCHANNEL(currentClient.getNickname(), channelName).c_str()));
				return ;
			}
			else
			{
				//check if the current user is channel operator
				if (!isExistingChannel->second.isChannelOp(currentClient) && isExistingChannel->second.getChannelOwner().getNickname() != currentClient.getNickname())
				{
					safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_CHANOPRIVSNEEDED(currentClient.getNickname(), channelName).c_str()));
					return ;
				}
				else
				{
					//check if the user to remove is in the channel
					isExistingUser = isExistingChannel->second.isAMember(userToRemove);
					if (_Channels[requestArguments[1]].hasMember(*isExistingUser))
					{
						//remove member
						std::string	messageToKicked;
						std::string messageToKicker = "User has successfully been removed !\r\n";
						safeSendMessage(currentClient.getSocket(), const_cast<char*>(messageToKicker.c_str()));
						
						//send an explanation to the user who has been kicked out
						if (requestArguments.size() > 3)
						{
							messageToKicked = "Reason for being kicked, according to Channel Operator ";
							std::size_t	found;
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
						isExistingChannel->second.removeMember(*isExistingUser);
					}
				}
			}
		}
	}
	else
		safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOTREGISTERED(currentClient.getNickname()).c_str()));
}

void	IrcServer::invite(std::vector<std::string> &requestArguments, User &currentClient)
{		
	//No need to verify if requestArguments[0] == "INVITE" -> it is done below (HANDLE COMMANDS)
	if (currentClient.isAuthentificated())
	{
		std::map<std::string, Channel>::iterator	isExistingChannel;
		std::vector<User>::iterator					isUserToInvite;
		std::string									userToInvite = requestArguments[1];
		std::string									channelName = requestArguments[2];
		bool										isMember;

		//check if channel exists
		isExistingChannel = _Channels.find(channelName);
		if (isExistingChannel != _Channels.end())
		{
			//check if the user inviting is in channel
			isMember = isExistingChannel->second.hasMember(currentClient);
			if (!isMember)
			{
				safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOTONCHANNEL(currentClient.getNickname(), channelName).c_str()));
				return ;
			}
			else
			{
				//check if userToInvite exists
				if (!_ConnectedUsers.userExists(userToInvite))
				{
					safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOSUCHNICK(currentClient.getNickname(), userToInvite).c_str()));
					return ;
				}
				else
				{
					//check if userToInvite already on channel
					isUserToInvite = isExistingChannel->second.isAMember(userToInvite);
					if (isUserToInvite != isExistingChannel->second.getMembersList().end())
					{
						safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_USERONCHANNEL(currentClient.getNickname(), userToInvite, channelName).c_str()));
						return ;
					}
					else
					{
						User *invitedUser =  _ConnectedUsers.getUser(userToInvite);
						//check channel's mode
						if (isExistingChannel->second.findMode("+i"))
						{
							//check that the current user is a channel operator
							if (!isExistingChannel->second.isChannelOp(currentClient))
							{
								safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_CHANOPRIVSNEEDED(currentClient.getNickname(), channelName).c_str()));
								return ;
							}
							else
							{
								safeSendMessage(currentClient.getSocket(), const_cast<char *>(RPL_INVITING(currentClient.getNickname(), userToInvite, channelName).c_str()));
								invitedUser->setChannelsList(channelName); //add this channel to the channel list the user has been invited to
								std::string	messageToInvitedUser = currentClient.getNickname() + " invited you to join channel " + requestArguments[2] + ".\r\n";
								safeSendMessage(invitedUser->getSocket(), const_cast<char*>(messageToInvitedUser.c_str()));
								return ;
							}
						}
						else
						{
							safeSendMessage(currentClient.getSocket(), const_cast<char *>(RPL_INVITING(currentClient.getNickname(), userToInvite, channelName).c_str()));
							invitedUser->setChannelsList(channelName); //add this channel to the channel list the user has been invited to
							std::string	messageToInvitedUser = currentClient.getNickname() + " invited you to join channel " + requestArguments[2] + ".\r\n";
							safeSendMessage(invitedUser->getSocket(), const_cast<char*>(messageToInvitedUser.c_str()));
							return ;
						}
					}
				}
			}
		}
		else
		{
			safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOSUCHCHANNEL(currentClient.getNickname(), channelName).c_str()));
			return ; 
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
		bool										isMember;
	
		//check if channel exists
		isExistingChannel = _Channels.find(channelName);
		if (isExistingChannel == _Channels.end())
		{
			safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOSUCHCHANNEL(currentClient.getNickname(), channelName).c_str()));
			return ; 
		}
		else
		{
			//it should just print out the channel's topic, if defined
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
			//it should update the topic, only if the user is a channel operator and the channel mode allows it
			if (requestArguments.size() == 3)
			{
				//check if user wanting to perform the change is in the channel
				isMember = isExistingChannel->second.hasMember(currentClient);
				if (!isMember)
				{
					safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOTONCHANNEL(currentClient.getNickname(), channelName).c_str()));
					return ;
				}
				else
				{
					//check if user is a channel operator
					if (!isExistingChannel->second.isChannelOp(currentClient))
					{
						safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_CHANOPRIVSNEEDED(currentClient.getNickname(), channelName).c_str()));
						return ;
					}
					else
					{
						//check if channel mode allows change of topic
						if (isExistingChannel->second.findMode("+t"))
						{
							isExistingChannel->second.setChannelTopic(requestArguments[2]);
							safeSendMessage(currentClient.getSocket(), const_cast<char *>(RPL_TOPIC(currentClient.getNickname(), channelName, isExistingChannel->second.getChannelTopic()).c_str()));
							return ;
						}
					}
				}
			}
		}
	}
	else
		safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOTREGISTERED(currentClient.getNickname()).c_str()));
}


void	IrcServer::dealWithSpecialModes(std::vector<std::string> &requestArguments, std::string &mode, std::map<std::string, Channel>::iterator channel, User &currentClient)
{
	std::size_t	found_o = mode.find('o');
	std::size_t	found_k = mode.find('k');

	//either +o, -o, +k, -k
	if (found_o != std::string::npos || found_k != std::string::npos)
	{
		if (requestArguments.size() != 4)
		{
			safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NEEDMOREPARAMS(currentClient.getNickname(), requestArguments[0]).c_str()));
			return ;
		}
		if (mode == "+o" || mode == "-o")
		{
			//check if the fourth argument (aka the user) exists !
			std::string	userToCheck = requestArguments[3];
			if (!_ConnectedUsers.userExists(userToCheck))
			{
				safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOSUCHNICK(currentClient.getNickname(), userToCheck).c_str()));
				return ;
			}
			else
			{
				std::vector<User>::iterator newChanOp;
				newChanOp = channel->second.isAMember(userToCheck);
				//if the user is in the channel
				if (mode == "+o" && newChanOp != channel->second.getMembersList().end())
				{
						channel->second.addOperator(*newChanOp);
						return ;
				}
				if (mode == "-o" && newChanOp != channel->second.getMembersList().end())
				{
						channel->second.removeOperator(*newChanOp);
						return ;
				}
			}
		}
		if (mode == "+k" || mode == "-k")
		{
			std::string	channelPassword = requestArguments[3];
			if (mode == "+k")
			{
				channel->second.setChannelPassword(channelPassword);
				//TEST BELOW
				std::cout << "Channel's password is: " << channel->second.getChannelPassword();
				return ;
			}
			else if (mode == "-k")
			{
				channel->second.removeChannelPassword();
				return ;
			}
		}
	}
}

int	IrcServer::modeWasFound(const std::vector<std::string> &currentMode, std::string &newMode)
{
	for (std::size_t i = 0; i < currentMode.size(); i++)
	{
		std::size_t	found = currentMode[i].find(newMode[1]);
		if (found != std::string::npos)
		{
			if (currentMode[i].compare(newMode) != 0) //if currentModes[i] is +i, newModes[y] is -i (are opposite)
				return (i);
			else 
				return (-1); //are the same
		}
	}
	return (-2); //not found, can be added to _modesList
}

void	IrcServer::compareModes(std::vector<std::string> &requestArguments, std::vector<std::string>	&newModes, std::map<std::string, Channel>::iterator channel, User &currentClient)
{

	const std::vector<std::string>	&currentModes = channel->second.getModesList();
	
	for (size_t j = 0; j < newModes.size(); j++)
	{
		int	ret = modeWasFound(currentModes, newModes[j]);
		//mode was not found in _modesList and must therefore be added
		if (ret == -2)
		{
			dealWithSpecialModes(requestArguments, newModes[j], channel, currentClient); //deal with o and k
			channel->second.addMode(newModes[j]);			
		}
		//the opposite mode of newMdoes[i] was found, need to change !
		if (ret > -1)
		{
			dealWithSpecialModes(requestArguments, newModes[j], channel, currentClient); //deal with o and k
			channel->second.changeMode(currentModes[ret], newModes[j]); //the mode set is the opposite of the new mode (-i changes to +i)
		}
		if (ret == - 1) //special case : no need to add or remove the mode - but need to update
			dealWithSpecialModes(requestArguments, newModes[j], channel, currentClient);
	}
}

void	IrcServer::sortModes(std::vector<std::string> &requestArguments, std::map<std::string, Channel>::iterator channel, User &currentClient)
{
	std::string					sign;
	std::string					mode;
	std::vector<std::string>	modesToSet;


	sign = requestArguments[2].substr(0, 1); //first character set as sign
	for (std::size_t i = 1; i < requestArguments[2].size(); i++)
	{
		if (requestArguments[2][i] != '-' && requestArguments[2][i] != '+')
		{
			mode = sign + requestArguments[2].substr(i, 1);
			modesToSet.push_back(mode);
		}
		else
			sign = requestArguments[2].substr(i, 1);
	}
	compareModes(requestArguments, modesToSet, channel, currentClient);
}

void	IrcServer::mode(std::vector<std::string> &requestArguments, User &currentClient)
{

	//No need to verify if requestArguments[0] == "MODE" -> it is done below (HANDLE COMMANDS)
	if (currentClient.isAuthentificated())
	{
		std::map<std::string, Channel>::iterator	isExistingChannel;
		std::string									channelName = requestArguments[1];
		bool										isMember;

		//check if channel exists
		isExistingChannel = _Channels.find(channelName);
		if (isExistingChannel == _Channels.end())
		{
			//Verify if channelName is a user and send a error numeric that MODE user is not supported
			if (_ConnectedUsers.getUser(channelName) != NULL)
				safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_UNKNOWNERROR(currentClient.getNickname(), "MODE " + channelName, "user mode is not supported").c_str()));
			//Channel doesn't exist
			else
				safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOSUCHCHANNEL(currentClient.getNickname(), channelName).c_str()));
			return ; 
		}
		else
		{
			//check if user wanting to perform the change is in channel
			isMember = isExistingChannel->second.hasMember(currentClient);
			if (!isMember)
			{
				safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOTONCHANNEL(currentClient.getNickname(), channelName).c_str()));
				return ;
			}
			else
			{
				//check if user is a channel operator
				if (!isExistingChannel->second.isChannelOp(currentClient))
				{
					safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_CHANOPRIVSNEEDED(currentClient.getNickname(), channelName).c_str()));
					return ;
				}
				else
				{
					//Let's perform the needed operations !
					sortModes(requestArguments, isExistingChannel, currentClient);				
					//TEST BELOW
					// const std::vector<std::string>	&currentModes = isExistingChannel->second.getModesList();
					// std::cout << "THE MODES ARE : " << std::endl;
					// for (size_t i  = 0; i < currentModes.size(); i++)
					// 	std::cout << currentModes[i] << std::endl;
				}
			}
		}
	}
	else
		safeSendMessage(currentClient.getSocket(), const_cast<char *>(ERR_NOTREGISTERED(currentClient.getNickname()).c_str()));
}
