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

IrcServer::IrcServer(const unsigned int &portNumber, const std::string& password) : _serverPort(portNumber),  _serverPassword(password), _serverFd(-1)
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
	
	//AUTHENTICATION PROTOTYPE
	//split request assuming this is ONE message (terminated by CRLF)
	//by rfc1459 there can only one command per message, but multiple messages per request
	std::stringstream request(buffer);
	std::string command;
	std::string argument;

	request >> command;
	request >> argument;
	//check command and argument
		//to add :
		//- if the user is already connected, reject PASS request
		//- figure out what rfc1459 means by only taking the last pass in terms of parsing
	if (command == "PASS")
	{
		if (argument == _serverPassword)
		{
			//delete user from lobby to connected users
			//set user authenticated status to true
			std::string message = "You're in ! Pick a nickname to start user the server.\r\n";
			safeSendMessage(clientFd, const_cast<char*>(message.c_str()));
		}
		else
		{
			std::string message = "Wrong password.\r\n";
			safeSendMessage(clientFd, const_cast<char*>(message.c_str()));
		}
	}
	//if command == anything while user has authenticated status, reject and remind to authenticate
	//if command == NICK
		// if user has a non-empty NICK update it, otherwise set nickname to a non-empty NICK, but reject if NICK is already taken, even by same user
	return;
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
