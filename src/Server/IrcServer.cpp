#include "IrcServer.hpp"

//EXTERN SIGNAL HANDLER__________________________________________________________________________________________________________________________
//Header-level client socket lists
std::vector<int> g_clientSockets;

//Signal handler for SIGINT, SIGTERM (kill signal) and SIGQUIT to avoid port-clogging on abrupt exit
void signalHandler(int signal)
{
	//UNCOMMENT THIS WHEN /EXIT IS IMPLEMENTED
	// if (signal == SIGINT)
	// {
	// 	std::cout << YELLOW << "SIGINT ignored, please type " << BWHITE << "/exit" << RESET << YELLOW << " to safely shutdown the server next time." << RESET << std::endl;
	// 	return ;
	// }
	// if (signal == SIGQUIT)
	// {
	// 	std::cout << YELLOW << "SIGQUIT ignored, please type " << BWHITE << "/exit" << RESET << YELLOW << " to safely shutdown the server next time." << RESET << std::endl;
	// 	return ;
	// }
	if (signal == SIGTERM || signal == SIGINT)
	{
		std::cout << YELLOW << "\n[IRC Server shutdown by SIGTERM Request, attempting graceful exit...]" << RESET << std::endl;
		for (size_t i = 0; i < g_clientSockets.size(); i++)
			close(g_clientSockets[i]);
		std::cout << TITLE << CLEARLINE << "[Server shutdown successful]" << RESET << std::endl;
		exit(EXIT_SUCCESS);
	}
}

//IRCSERVER CLASS______________________________________________________________________________________________________________________________________
//IrcServer can't be instantiated using the default constructor
IrcServer::IrcServer()
{}

IrcServer::IrcServer(const unsigned int &portNumber, const std::string& password) : _serverFd(-1), _serverPort(portNumber),  _serverPassword(password)
{
	//DEFINE SIGHANDLERS
	std::signal(SIGINT, signalHandler);
	std::signal(SIGQUIT, signalHandler);
	std::signal(SIGTERM, signalHandler);
	try
	{
		if ((_serverFd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		{
			throw SocketCreationException();
		}
		//NAMING SOCKET
		_serverSockAddr.sin_family = AF_INET;
		_serverSockAddr.sin_addr.s_addr = INADDR_ANY;
		_serverSockAddr.sin_port = htons(_serverPort);
		memset(_serverSockAddr.sin_zero, 0, sizeof(_serverSockAddr.sin_zero));
		if (bind(_serverFd, (struct sockaddr *)&_serverSockAddr, sizeof(_serverSockAddr)) == -1)
		{
			close(_serverFd);
			throw BindException();
		}
		//LISTEN PORT WITH BACKLOG ENABLED
		if (listen(_serverFd, SOMAXCONN) == -1)
			throw ListenException();
		//ADDED SERVERFD TO FD LIST FOR CLEANUP
		g_clientSockets.push_back(_serverFd);
	}
	catch (const IrcServerException& err)
	{
		std::cerr << err.what() << std::endl;
		exit(EXIT_FAILURE);
	}
}

IrcServer::~IrcServer()
{
	for (size_t i = 0; i < g_clientSockets.size(); i++)
		close(g_clientSockets[i]);
	std::cout << TITLE << CLEARLINE << "[Server shutdown successful]" << RESET << std::endl;
}

IrcServer::IrcServer(const IrcServer &cpy)
{
	if (this != &cpy)
		*this = cpy;
}

IrcServer &IrcServer::operator=(const IrcServer &cpy)
{
	if (this != &cpy)
	{
		_serverSockAddr = cpy._serverSockAddr;
		_serverPort = cpy._serverPort;
		_serverPassword = cpy._serverPassword;
		_serverFd = cpy._serverFd;
		_ConnectedUsers = cpy._ConnectedUsers;
		_ConnectedUsersMap = cpy._ConnectedUsersMap;
		_Channels = cpy._Channels;
	}
	return *this;
}

void IrcServer::clearFdFromList(int clientFd)
{
	int j = 0;
	for (unsigned int i = 0; i < g_clientSockets.size(); i++)
	{
		if (clientFd == g_clientSockets[i])
		{
			FD_CLR(clientFd, &_clientsFdSet);
			g_clientSockets.erase(g_clientSockets.begin() + j);
			_serverResponses.erase(clientFd);
			this->_ConnectedUsers.removeUser(clientFd);
			close(clientFd);
			return ;
		}
		j++;
	}
}

void IrcServer::handleRequest(int clientFd)
{
	char	buffer[MESSAGE_BUFFER_SIZE] = {0};
	int		bytes_received;

	bytes_received = recv(clientFd, buffer, MESSAGE_BUFFER_SIZE, 0);
	if (bytes_received == -1)
	{
		clearFdFromList(clientFd);
		return;
	}
	if (bytes_received == 0)
	{
		std::cout << Utils::getLocalTime() << "Client [" << clientFd << "] disconnected." << std::endl;
		clearFdFromList(clientFd);
		return;
	}
	printSocketData(clientFd, buffer);
	
	//REFACTOR INTO METHODS
	//AUTHENTICATION PROTOTYPE___________________________________________________________________________________
	std::stringstream request(buffer);
	std::string command;
	std::string argument;

	request >> command;
	request >> argument;

	User *user = _ConnectedUsers.getUser(clientFd);
	//PASS COMMAND
	//IF the command is PASS, and it has an argument, and the client hasn't logged in yet
	if (command == "PASS" && !argument.empty() && !user->hasPassword())
	{
		if (argument == _serverPassword)
		{
			user->setHasPassword(true);
			std::string message = "You're in ! Pick a nickname to start user the server.\r\n";
			safeSendMessage(clientFd, const_cast<char*>(message.c_str()));
		}
		else
		{
			std::string message = "Wrong password.\r\n";
			safeSendMessage(clientFd, const_cast<char*>(message.c_str()));
			return ;
		}
	}
	//IF the client is already logged in
	else
	{
		if (command == "PASS" && !argument.empty() && user->hasPassword())
		{
			std::string message = "You're already logged in.\r\n";
			safeSendMessage(clientFd, const_cast<char*>(message.c_str()));
			return ;
		}
	}
	//NICK COMMAND
	//IF the command is nick, and there is an argument, and the user is logged in
	if (command == "NICK" && !argument.empty() && user->hasPassword())
	{
		//Check if the user is known
		User *isKnownUser = _ConnectedUsers.getUser(argument);
		//IF if its a known user, check if it is THIS client or another client
		if (isKnownUser)
		{
			if (isKnownUser != user)
			{
				std::string message = "Sorry! This nickname is already taken.\r\n";
				safeSendMessage(clientFd, const_cast<char*>(message.c_str()));
				return ;
			}
			else
			{
				std::string message = "Your nickname has already been set.\r\n";
				safeSendMessage(clientFd, const_cast<char*>(message.c_str()));
				return ;
			}
		}
		//IF the user is not known, check if it already has a nickname (update), otherwise set it
		else
		{
			if (user == _ConnectedUsers.getUser(clientFd) && user->getNickname() != "")
			{
				this->_ConnectedUsers.linkUserToNickname(argument, clientFd);
				user->setNickname(argument);
				std::string message = "Your username has been updated to [" + user->getNickname() + "].\r\n";
				safeSendMessage(clientFd, const_cast<char*>(message.c_str()));
				return ;
			}
			else
			{
				this->_ConnectedUsers.linkUserToNickname(argument, clientFd);
				// set nickname inside of linkUserToNickname ???
				user->setNickname(argument);
				std::string message = "Hello " + user->getNickname() + "! You are now fully authenticated.\r\n";
				safeSendMessage(clientFd, const_cast<char*>(message.c_str()));
			}
		}
	}
	//COMMAND WITHOUT PASS
	else
	{
		if (command != "PASS" && !argument.empty() && !user->hasPassword())
		{
			std::string message = "Please enter the server password first.\r\n";
			safeSendMessage(clientFd, const_cast<char*>(message.c_str()));
			return ;
		}
	}
	return;
	//AUTHENTICATION PROTOTYPE___________________________________________________________________________________
}

void IrcServer::run()
{
	FD_ZERO(&_clientsFdSet);
	FD_SET(_serverFd, &_clientsFdSet);

	while (true)
	{
		fd_set tmpSet = _clientsFdSet;
		if (select(FD_SETSIZE, &tmpSet, NULL, NULL, NULL) == -1)
		{
			std::cerr << "Error : Problem with file descriptor set." << std::endl;
		}
		if (FD_ISSET(_serverFd, &tmpSet))
		{
			if (acceptClient() == -1)
				std::cerr << "Error : Couldn't accept client." << std::endl;
		} 
		else 
		{
			for (unsigned int i = 0; i < g_clientSockets.size(); i++)
			{
				if (FD_ISSET(g_clientSockets[i], &tmpSet))
					handleRequest(g_clientSockets[i]);
			}
		}
		// Handle server responses
		std::map<int, std::string>::iterator it;
		for (it = _serverResponses.begin(); it != _serverResponses.end(); ++it)
		{
			int					clientFd = it->first;
			const std::string&	message = it->second;
			if (!message.empty())
			{
				safeSendMessage(clientFd, const_cast<char*>(message.c_str()));
				it->second = ""; // Clear the message after sending
			}
		}
	}
}
