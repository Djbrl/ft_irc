#ifndef __IRC_SERVER_HPP__
# define __IRC_SERVER_HPP__

# include "_defines.hpp"
# include "AServer.hpp"
# include "Channel.hpp"
# include "User.hpp"
# include "UserMap.hpp"

//IRCSERVER CLASS____________________________________________________________________________________________________
//IrcServer inherits from AServer, and carries all the methods and attributes needed for our IRC Server 

class IrcServer : public AServer
{
	private:
	int									_serverFd;
	unsigned int						_serverPort;
	std::string							_serverPassword;
	sockaddr_in_t						_serverSockAddr;
	std::map<int, std::string>			_serverResponses;
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
	int									acceptClient();
	void								dsy_cbarbit_AuthAndChannelMethodsPrototype(int clientFd, char *buffer);


	//CHANNEL
	void								addChannel(const std::string &channelName, User &owner);
	void								removeChannel(const std::string &channelName);

	//PARSING
	void								parseQuery(int clientFd, std::string clientQuery);
	void								dispatchQuery(int clientFd, std::string clientQuery);
	void    							passCommand(int clientFd, std::string passCommand, std::stringstream &commandCopy);
	std::string     					parsePassCommand(int clientFd, std::stringstream &commandCopy);

	//PROCESS
	void								sendWelcomeMessage(int clientSocket);
	void   								safeSendMessage(int targeted_client, char *msg);
	void   								sendServerResponse(int sender_fd, char *msg);
	void								handleRequest(int clientFd);
	
	//UTILS
	void    							clearFdFromList(int client_fd);
	void								printSocketData(int clientSocket, char *socketData);
	//GETTERS__________________________________________________________________________________________________
	//SETTERS__________________________________________________________________________________________________
};

#endif
