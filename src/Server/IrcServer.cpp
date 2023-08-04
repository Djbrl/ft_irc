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

//Mandatory constructor, protected with try-catches
IrcServer::IrcServer(const unsigned int &portNumber, const std::string& password) : _serverPort(portNumber),  _serverPassword(password), _serverFd(-1)
{
	//DEFINE SIGHANDLERS
	std::signal(SIGINT, signalHandler);
	std::signal(SIGQUIT, signalHandler);
	std::signal(SIGTERM, signalHandler);
	try
	{
	if ((_serverFd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		throw SocketCreationException();
		//NAMING SOCKET
		_serverSockAddr.sin_family = AF_INET;
		_serverSockAddr.sin_addr.s_addr = INADDR_ANY;
		_serverSockAddr.sin_port = htons(_serverPort);
		std::memset(_serverSockAddr.sin_zero, 0, sizeof(_serverSockAddr.sin_zero));
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

void    IrcServer::clearFdFromList(int clientFd)
{
    int j = 0;
    for (unsigned int i = 0; i < g_clientSockets.size(); i++)
    {
        if (clientFd == g_clientSockets[i])
		{
			FD_CLR(clientFd, &_clientsFdSet);
            g_clientSockets.erase(g_clientSockets.begin()+j);
			close(clientFd);
		}
		j++;
    }
}

int IrcServer::handleRequest(int clientFd)
{
    char    buffer[MESSAGE_BUFFER_SIZE] = {0};
    int     bytes_received;

	bytes_received = recv(clientFd, buffer, MESSAGE_BUFFER_SIZE, 0);
    if (bytes_received == -1) 
	{
        clearFdFromList(clientFd);
		// SEND BACK NUMERIC REPLY TO CLIENT 		//
		/* sendMessage(new_sockfd, welcome_msg);	*/
		// SEND BACK NUMERIC REPLY TO CLIENT 		//
		return (-1);
    }
    if (bytes_received == 0)
    {
		std::cout << Utils::getLocalTime() << "Client [" << clientFd << "] disconnected." << std::endl;
        clearFdFromList(clientFd);
		// SEND BACK NUMERIC REPLY TO CLIENT 		//
		/* sendMessage(new_sockfd, welcome_msg);	*/
		// SEND BACK NUMERIC REPLY TO CLIENT 		//
        return (0);
    }
	printSocketData(clientFd, buffer);
	// SEND BACK NUMERIC REPLY TO CLIENT 		//
	/* sendMessage(clientFd, welcome_msg);		*/
	// SEND BACK NUMERIC REPLY TO CLIENT 		//
	return (0);
}

void IrcServer::run()
{
	std::istringstream	requestField;
	std::string			requestStatus;

	FD_ZERO(&_clientsFdSet);
    FD_SET(_serverFd, &_clientsFdSet);
	while (true)
	{
		fd_set	tmpSet = _clientsFdSet;
		if (select(FD_SETSIZE, &tmpSet, NULL, NULL, NULL) == -1)
        {
            std::cerr << "Error : Problem with file descriptor set." << std::endl;
        }
        if (FD_ISSET(_serverFd, &tmpSet))
        {
            if (acceptClient() == -1)
            {
	        	std::cerr << "Error : Couldn't accept client." << std::endl;
			}
        }
        else
        {
            for (unsigned int i = 0; i < g_clientSockets.size(); i++)
            {
                if (FD_ISSET(g_clientSockets[i], &tmpSet))
                {    
                    if (handleRequest(g_clientSockets[i]) == -1)
                    {
		          		std::cerr << "Error : Couldn't handle request." << std::endl;
					}
                }
            }
        }
	}
}
