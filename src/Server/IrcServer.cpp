#include "IrcServer.hpp"

//EXTERN SIGNAL HANDLER__________________________________________________________________________________________________________________________
//Header-level client socket lists
std::vector<int>	g_clientSockets;
bool				requestShutdown;

//Signal handler for SIGINT, SIGTERM (kill signal) and SIGQUIT to avoid port-clogging on abrupt exit
void signalHandler(int signal)
{
	if (signal == SIGTERM)
	{
		std::cout << RED << "\n[IRC Server shutdown by SIGTERM Request, closing connections...]" << RESET << std::endl;
		for (size_t i = 0; i < g_clientSockets.size(); i++)
		{
			close(g_clientSockets[i]);
			std::cout << Utils::getLocalTime() << "Connection to client [" << g_clientSockets[i] << "] closed." << std::endl;
		}
		requestShutdown = true;
	}
	else if (signal == SIGINT)
	{
		std::cout << YELLOW << "\n[IRC Server shutdown by SIGINT Request, closing connections...]" << RESET << std::endl;
		for (size_t i = 0; i < g_clientSockets.size(); i++)
		{
			close(g_clientSockets[i]);
			std::cout << Utils::getLocalTime() << "Connection to client [" << g_clientSockets[i] << "] closed." << std::endl;
		}
		requestShutdown = true;
	}
	else if (signal == SIGPIPE)
		std::cout << "Warning: SIGPIPE request from system ignored." << std::endl;
	return ;
}

//IRCSERVER CLASS______________________________________________________________________________________________________________________________________
//IrcServer can't be instantiated using the default constructor
IrcServer::IrcServer()
{
}

IrcServer::IrcServer(const unsigned int &portNumber, const std::string& password) : _serverFd(-1), _serverPort(portNumber),  _serverPassword(password), _serverCreationDate(time(NULL))
{
	//DEFINE SIGHANDLERS
	std::signal(SIGINT, signalHandler);
	std::signal(SIGQUIT, signalHandler);
	std::signal(SIGTERM, signalHandler);
	std::signal(SIGPIPE, signalHandler);
	requestShutdown = false;
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
		
		//Prevent BindException when restarting the server
		int opt = 1;
		setsockopt(_serverFd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

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
	std::cout << TITLE << CLEARLINE << "[Server shutdown successful]" << RESET << "\n" << std::endl;
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
		_serverFd = cpy._serverFd;
		_serverPort = cpy._serverPort;
		_serverPassword = cpy._serverPassword;
		_serverSockAddr = cpy._serverSockAddr;
		_clientsFdSet = cpy._clientsFdSet;
		_ConnectedUsers = cpy._ConnectedUsers;
		_Channels = cpy._Channels;
	}
	return *this;
}

//METHODS________________________________________________________________________________________________________________________________________________

void IrcServer::run()
{
	FD_ZERO(&_clientsFdSet);
	FD_SET(_serverFd, &_clientsFdSet);

	while (requestShutdown != true)
	{
		fd_set tmpSet = _clientsFdSet;
		if (select(FD_SETSIZE, &tmpSet, NULL, NULL, NULL) == -1)
		{
			if (requestShutdown == true)
				break ;
			else
				std::cerr << "Error: select() failed." << std::endl;
		}
		if (FD_ISSET(_serverFd, &tmpSet))
			acceptClient();
		else 
		{
			for (unsigned int i = 0; i < g_clientSockets.size(); i++)
			{
				if (FD_ISSET(g_clientSockets[i], &tmpSet))
					handleRequest(g_clientSockets[i]);
			}
		}
	}
	return ;
}

void	IrcServer::acceptClient()
{
	std::string	clientIP;
	sockaddr	clientSockAddr;
	int			clientAddrLen;
	int			dataSocketFd;

	try
	{
		clientAddrLen = sizeof(clientSockAddr);
		if ((dataSocketFd = accept(_serverFd, (struct sockaddr*)&clientSockAddr, (socklen_t*)&clientAddrLen)) == -1)
			throw AcceptException();
		FD_SET(dataSocketFd, &_clientsFdSet);
		g_clientSockets.push_back(dataSocketFd);
		clientIP = inet_ntoa(((struct sockaddr_in*)&clientSockAddr)->sin_addr);
		std::cout << Utils::getLocalTime() << "New client connection: [" << dataSocketFd << "] - " << BWHITE << clientIP << RESET << "." << std::endl;
		_ConnectedUsers.addUser(dataSocketFd);
		sendWelcomeMessage(dataSocketFd);
	}
	catch (const AcceptException& e)
	{
		std::cerr << e.what() << '\n';
	}
	return ;
}
void IrcServer::handleSuddenDisconnection(int clientFd)
{
		std::cout << Utils::getLocalTime() << "Client [" << clientFd << "] has left the server." << std::endl;
		//NOTIFY USERS
		std::map<std::string, Channel>::iterator	it = _Channels.begin();
		User										*userToRemove = _ConnectedUsers.getUser(clientFd);		
		if (userToRemove != NULL)
		{
			while (it != _Channels.end())
			{
				if (it->second.hasMember(*userToRemove))
				{
					std::string quitMessage = "has left the server (reason :sudden disconnection)";
					it->second.sendMessageToUsers(quitMessage, userToRemove->getNickname());
				}
				it++;
			}
		}
		//DELETE USER
		disconnectUserFromServer(clientFd);
		//DELETE CHANNELS THAT WILL BECOME EMPTY
		std::vector<std::string> channelsToDelete;
		for (std::map<std::string, Channel>::iterator it = _Channels.begin(); it != _Channels.end(); it++)
		{
			if (it->second.getMembersList().size() == 0)
			{
				//delayed delete to avoid segfault
				channelsToDelete.push_back(it->first);
				std::string noticeMessage = "NOTICE broadcast :" + it->first + " has been removed for inactivity" + "\r\n";
				_ConnectedUsers.broadcastMessage(const_cast<char *>(noticeMessage.c_str()));
			}
		}
		for (size_t i = 0; i < channelsToDelete.size(); i++)
			_Channels.erase(channelsToDelete[i]);
		return ;
}
void IrcServer::handleRequest(int clientFd)
{
	char	buffer[MESSAGE_BUFFER_SIZE] = {0};
	ssize_t		bytes_received;

	memset(buffer, 0, sizeof(buffer));

	//UNEXPECTED DISCONNECTION
	bytes_received = recv(clientFd, buffer, MESSAGE_BUFFER_SIZE, 0);
	if (bytes_received <= 0)
	{
		handleSuddenDisconnection(clientFd);
		return ;
	}

	User *current_user = _ConnectedUsers.getUser(clientFd);
	if (!current_user)
		return ;

	char *ubuffer = current_user->buffer;
	char *position = std::find(ubuffer, ubuffer + MESSAGE_BUFFER_SIZE, '\0');

	size_t buffer_size_left = MESSAGE_BUFFER_SIZE - std::distance(ubuffer, position);

	size_t n_to_copy = std::min(size_t(bytes_received), buffer_size_left);
	memcpy(position, buffer, n_to_copy);
    // parseQuery(clientFd, buffer);
    //AUTHENTICATION PROTOTYPE___________________________________________________________________________________
	
	// std::cout << "buffer :" << buffer << std::endl;
	// std::cout << "user buffer :" << current_user->buffer << std::endl;
	
	if (buffer_size_left == 0 && std::string(buffer, 512).find("\r\n", 0) == std::string::npos)
		return ;
	if (buffer_size_left != 0 && std::string(ubuffer, 512).find("\r\n", 0) == std::string::npos)
		return ;

	if (buffer_size_left == 0 && std::string(buffer, 512).find("\r\n", 0) != std::string::npos)
	{
		ubuffer[510] = '\r';
		ubuffer[511] = '\n';
	}
	std::vector<std::string> requests = splitStringByCRLF(std::string(ubuffer, 512), ubuffer);
	for (int i = 0; i < (int)requests.size(); i++)
	{
		std::cout << Utils::getLocalTime() << "Received request [" << requests[i] << "] from " << "[" << clientFd << "]" << std::endl;
		try
		{
			std::vector<std::string> args = parse_message(requests[i]);
			dsy_cbarbit_AuthAndChannelMethodsPrototype(clientFd, args);
		}
		catch(const std::exception& e)
		{
			std::cerr << "Error parsing command: " << e.what() << '\n';
		}
	}
	//AUTHENTICATION PROTOTYPE___________________________________________________________________________________
	return ;
}