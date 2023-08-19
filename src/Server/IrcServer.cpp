#include "IrcServer.hpp"

//EXTERN SIGNAL HANDLER__________________________________________________________________________________________________________________________
//Header-level client socket lists
std::vector<int> g_clientSockets;

//Signal handler for SIGINT, SIGTERM (kill signal) and SIGQUIT to avoid port-clogging on abrupt exit
void signalHandler(int signal)
{
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

IrcServer::IrcServer(const unsigned int &portNumber, const std::string& password) : _serverFd(-1), _serverPort(portNumber),  _serverPassword(password), _serverCreationDate(time(NULL))
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
	for (size_t i = 0; i < g_clientSockets.size(); i++)
		clearFdFromList(g_clientSockets[i]);
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

	while (true)
	{
		fd_set tmpSet = _clientsFdSet;
		if (select(FD_SETSIZE, &tmpSet, NULL, NULL, NULL) == -1)
			std::cerr << "Error : Problem with file descriptor set." << std::endl;
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

void IrcServer::handleRequest(int clientFd)
{
	char	buffer[MESSAGE_BUFFER_SIZE] = {0};
	ssize_t		bytes_received;

	memset(buffer, 0, sizeof(buffer));
	bytes_received = recv(clientFd, buffer, MESSAGE_BUFFER_SIZE, 0);
	if (bytes_received <= 0)
	{
		std::cout << Utils::getLocalTime() << "Client [" << clientFd << "] disconnected." << std::endl;
		clearFdFromList(clientFd);
		return;
	}
	User *current_user = _ConnectedUsers.getUser(clientFd);
	if (!current_user)
		throw std::exception();
	
	char *ubuffer = current_user->buffer;
	char *position = std::find(ubuffer, ubuffer + MESSAGE_BUFFER_SIZE, '\0');

	size_t buffer_size_left = MESSAGE_BUFFER_SIZE - std::distance(ubuffer, position);

	size_t n_to_copy = std::min(size_t(bytes_received), buffer_size_left);
	memcpy(position, buffer, n_to_copy);
    // parseQuery(clientFd, buffer);
    //AUTHENTICATION PROTOTYPE___________________________________________________________________________________
	
	std::cout << "buffer :" << buffer << std::endl;
	std::cout << "user buffer :" << current_user->buffer << std::endl;
	
	if (buffer_size_left == 0 && std::string(buffer).find("\r\n", 0) == std::string::npos)
		return ;
	if (buffer_size_left != 0 && std::string(ubuffer).find("\r\n", 0) == std::string::npos)
		return ;
	std::vector<std::string> requests = splitStringByCRLF(std::string(ubuffer), ubuffer);
	for (int i = 0; i < (int)requests.size(); i++)
	{
		std::cout << "command in queue [" << requests[i] << "]" << std::endl;
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