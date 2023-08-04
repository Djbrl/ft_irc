#ifndef __IRC_SERVER_HPP__
# define __IRC_SERVER_HPP__

# include "_defines.hpp"
# include "AServer.hpp"
# include "Channel.hpp"
# include "User.hpp"

//IRCSERVER CLASS____________________________________________________________________________________________________
//IrcServer inherits from AServer, and carries all the methods and attributes needed for our IRC Server 

class IrcServer : public AServer
{
	private:
	sockaddr_in_t						_serverSockAddr;
	unsigned int						_serverPort;
	std::string							_serverPassword;
	int									_serverFd;
	std::map<std::string, User>			_ConnectedUsers;
	std::map<std::string, Channel>		_Channels;
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
	//camille
	void    decorticateMessage(int targeted_client, char *msg);
	void    sendMessage(int sender_fd, char *msg);
	void    clearFdFromList(int client_fd);
	int new_client();
	int handleRequest(int client_fd);
										//authenticateClient
										//createChannel
											//Channel related methods...
										//loginUser
											//User related methods...
	//GETTERS__________________________________________________________________________________________________
	//SETTERS__________________________________________________________________________________________________
};

#endif
