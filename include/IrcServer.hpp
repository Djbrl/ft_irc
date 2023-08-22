#ifndef __IRC_SERVER_HPP__
# define __IRC_SERVER_HPP__

# include "_defines.hpp"
# include "AServer.hpp"
# include "Channel.hpp"
# include "User.hpp"
# include "UserMap.hpp"
# include "CommandParsing.hpp"

//IRCSERVER CLASS____________________________________________________________________________________________________
//IrcServer inherits from AServer, and carries all the methods and attributes needed for our IRC Server 

class IrcServer : public AServer
{
	private:
	int									_serverFd;
	unsigned int						_serverPort;
	std::string							_serverPassword;
	sockaddr_in_t						_serverSockAddr;
	time_t								_serverCreationDate;
	std::map<std::string, Channel>		_Channels;
	fd_set								_clientsFdSet;
	UserMap							    _ConnectedUsers;
										IrcServer();
	public:
										~IrcServer();
										IrcServer(const unsigned int& portNumber, const std::string& password);
										IrcServer(const IrcServer &cpy);
	IrcServer							&operator=(const IrcServer &cpy);
	
	//METHODS__________________________________________________________________________________________________
	
	//CONNECTION
	void								run();
	void								acceptClient();
	
	//PROTOTYPE
	void								dsy_cbarbit_AuthAndChannelMethodsPrototype(int clientFd, std::vector<std::string>);
	void								capls(std::vector<std::string> &requestArguments, User &currentClient);
	void								pass(std::vector<std::string> &requestArguments, User &currentClient);
	void								nick(std::vector<std::string> &requestArguments, User &currentClient);
	void								user(std::vector<std::string> &requestArguments, User &currentClient);
	void								join(std::vector<std::string> &requestArguments, User &currentClient);
	void								list(std::vector<std::string> &requestArguments, User &currentClient);
	void								part(std::vector<std::string> &requestArguments, User &currentClient);
	void								quit(std::vector<std::string> &requestArguments, User &currentClient);
	// void								who(std::vector<std::string> &requestArguments, User &currentClient);
	void								privmsg(std::vector<std::string> &requestArguments, User &currentClient);
	void								notice(std::vector<std::string> &requestArguments, User &currentClient);
	void								pong(std::vector<std::string> &requestArguments, User &currentClient);
	void								kick(std::vector<std::string> &requestArguments, User &currentClient);
	void								invite(std::vector<std::string> &requestArguments, User &currentClient);
	void								topic(std::vector<std::string> &requestArguments, User &currentClient);
	void								mode(std::vector<std::string> &requestArguments, User &currentClient);
	void								sortModes(std::vector<std::string> &requestArguments, std::map<std::string, Channel>::iterator channel, User &currentClient);
	void								compareModes(std::vector<std::string> &requestArguments, std::vector<std::string>	&newModes, std::map<std::string, Channel>::iterator channel, User &currentClient);
	void								dealWithSpecialModes(std::vector<std::string> &requestArguments, std::string &specialMode, std::map<std::string, Channel>::iterator channel, User &currentClient);
	void								createChannel(const std::string &channelName, User &currentClient);
	void								joinChannel(const std::string &channelName, User &currentClient);
	int									modeWasFound(const std::vector<std::string> &currentMode, std::string &newMode);
	int									checkChannelExceptions(std::map<std::string, Channel>::iterator	&channel, std::vector<std::string> passwords, std::size_t channelIndex, User &currentClient);
	int									checkChannelChar(const char *channel);
	std::vector<std::string>			splitJoinArgument(std::string &argument);
	std::vector<std::string>			parseChannels(std::vector<std::string> channels, User &currentClient);
	void								handleSuddenDisconnection(int clientFd);
	//CHANNEL
	void								transferOwnership(Channel &chan);
	void								addChannel(const std::string &channelName, User &owner);
	void								removeChannel(const std::string &channelName);	
	void								updateMemberInChannels(std::string &oldNick, User &target);

	//PARSING
	void								parseQuery(int clientFd, std::string clientQuery);
	void								dispatchQuery(int clientFd, std::string clientQuery);
	void    							passCommand(int clientFd, std::string passCommand, std::stringstream &commandCopy);
	std::string     					parsePassCommand(int clientFd, std::stringstream &commandCopy);

	//PROCESS
	void								sendWelcomeMessage(int clientSocket);
	void   								safeSendMessage(int targeted_client, std::string msg);
	void								handleRequest(int clientFd);
	
	//UTILS
	void    							disconnectUserFromServer(int client_fd);
	std::vector<std::string>			splitStringByCRLF(const std::string &socketData, char *buffer);
	void								printSocketData(int clientSocket, char *socketData);
	//GETTERS__________________________________________________________________________________________________
	//SETTERS__________________________________________________________________________________________________
};

#endif
