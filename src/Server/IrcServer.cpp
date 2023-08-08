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
		_serverFd = cpy._serverFd;
		_serverPort = cpy._serverPort;
		_serverPassword = cpy._serverPassword;
		_serverSockAddr = cpy._serverSockAddr;
		_serverResponses = cpy._serverResponses;
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
				{
					handleRequest(g_clientSockets[i]);
				}
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

void	IrcServer::handleCAPLS(int clientFd)
{
	std::string CAPLS = "CAP * LS :PASS NICK JOIN PRIVMSG PONG\r\n";
	safeSendMessage(clientFd, const_cast<char *>(CAPLS.c_str()));	
	return ;
}

int	IrcServer::acceptClient()
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
		std::cout << Utils::getLocalTime() << "New client connection: [" << dataSocketFd << "] - " << BYELLOW << clientIP << RESET << "." << std::endl;
		char buf[MESSAGE_BUFFER_SIZE];
		recv(dataSocketFd, buf, MESSAGE_BUFFER_SIZE, 0);
		std::vector<std::string> request = splitStringByCRLF(std::string(buf));
		std::stringstream iss(request[0]);
		std::string word;
		iss >> word;
		if (word == "CAP")
			handleCAPLS(dataSocketFd);
		sendWelcomeMessage(dataSocketFd);
		this->_ConnectedUsers.addUser(dataSocketFd);
	} catch (const AcceptException& e) {
		std::cerr << e.what() << '\n';
	}
	return dataSocketFd;
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
    // parseQuery(clientFd, buffer);
    //AUTHENTICATION PROTOTYPE___________________________________________________________________________________
	std::vector<std::string> requests = splitStringByCRLF(buffer);
	for (int i = 0; i < (int)requests.size(); i++)
	{
		std::cout << "command in queue : " << requests[i] << "\n";
		dsy_cbarbit_AuthAndChannelMethodsPrototype(clientFd, const_cast<char *>(requests[i].c_str()));
	}
	//AUTHENTICATION PROTOTYPE___________________________________________________________________________________
	return ;
}