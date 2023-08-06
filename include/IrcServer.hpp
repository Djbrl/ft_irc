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
	UserMap							    _ConnectedUsers;
	sockaddr_in_t						_serverSockAddr;
	std::map<std::string, Channel>		_Channels;
	std::map<std::string, User>			_ConnectedUsersMap;
	std::map<int, std::string>			_serverResponses;
	fd_set								_clientsFdSet;
										IrcServer();
	public:
										~IrcServer();
										IrcServer(const unsigned int& portNumber, const std::string& password);
										IrcServer(const IrcServer &cpy);
	IrcServer							&operator=(const IrcServer &cpy);
	//METHODS__________________________________________________________________________________________________
	void								run();
	int									acceptClient();
	void								printSocketData(int clientSocket, char *socketData);
	void								handleClientWrite(int clientSocket);
	void								sendWelcomeMessage(int clientSocket);
	void   								safeSendMessage(int targeted_client, char *msg);
	void   								sendServerResponse(int sender_fd, char *msg);
	void    							clearFdFromList(int client_fd);
	void								handleRequest(int clientFd);
										//authenticateClient
										//createChannel
											//Channel related methods...
										//loginUser
											//User related methods...
	//GETTERS__________________________________________________________________________________________________
	User								*getUser(int socket_fd);
	User								*getUser(std::string &nickname);

	//SETTERS__________________________________________________________________________________________________
};

#endif
