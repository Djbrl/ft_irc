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
IrcServer::IrcServer(const std::string &portNumber, const std::string& password) : _serverPort(0),  _serverPassword(password), _serverFd(-1)
{
	//DEFINE SIGHANDLERS
	std::signal(SIGINT, signalHandler);
	std::signal(SIGQUIT, signalHandler);
	std::signal(SIGTERM, signalHandler);
	try
	{
		//PARSING PORT
		_serverPort = atoi(portNumber.c_str());
		if ((_serverPort == 0 && portNumber != "0") || _serverPort < 0)
			throw InvalidPortException();
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
		if (listen(_serverFd, QUEUE_BACKLOG) == -1)
			throw ListenException();
		//ADDED SERVERFD TO FD LIST FOR CLEANUP
		g_clientSockets.push_back(_serverFd);
	} catch (const InvalidPortException& err) {
		std::cerr << err.what() << std::endl;
		exit(EXIT_FAILURE);
	} catch (const SocketCreationException& err) {
		std::cerr << err.what() << std::endl;
		exit(EXIT_FAILURE);
	} catch (const BindException& err) {
		std::cerr << err.what() << std::endl;
		exit(EXIT_FAILURE);
	} catch (const ListenException& err) {
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

// void IrcServer::run()
// {
//     fd_set readfds;
//     int maxFd; // Store the highest file descriptor

//     while (true)
//     {
//         FD_ZERO(&readfds);
//         FD_SET(STDIN_FILENO, &readfds); // Add stdin (fd 0) to the set of file descriptors
//         FD_SET(_serverFd, &readfds);    // Add the server socket to the set of file descriptors

//         maxFd = std::max(STDIN_FILENO, _serverFd);

//         // Add all connected client sockets to the set of file descriptors
//         for (int clientSocket : g_clientSockets)
//         {
//             FD_SET(clientSocket, &readfds);
//             maxFd = std::max(maxFd, clientSocket);
//         }

//         // Wait for activity on any of the file descriptors
//         int activity = select(maxFd + 1, &readfds, nullptr, nullptr, nullptr);

//         if (activity < 0)
//         {
//             // Handle error
//             // You can add appropriate error handling here
//         }
//         else if (activity > 0)
//         {
//             // Check for activity on the server socket
//             if (FD_ISSET(_serverFd, &readfds))
//             {
//                 int dataSocketFd = acceptClient();
//                 g_clientSockets.push_back(dataSocketFd);
//             }

//             // Check for activity on client sockets
//             for (auto it = g_clientSockets.begin(); it != g_clientSockets.end(); ++it)
//             {
//                 int clientSocket = *it;
//                 if (FD_ISSET(clientSocket, &readfds))
//                 {
//                     std::istringstream requestField = readData(clientSocket);
//                     displayClientInfo(clientSocket);
//                     processCommand(requestField, clientSocket);
//                 }
//             }

//             // Check for activity on stdin (terminal input)
//             if (FD_ISSET(STDIN_FILENO, &readfds))
//             {
//                 std::string localCommand;
//                 std::getline(std::cin, localCommand);
// 				if (localCommand == "/exit")
// 					exit(EXIT_SUCCESS);
//             }
//         }
//     }
// }

//Server loop function :
//-Accept connections
//-Display information
//-Process the data
void IrcServer::run()
{
	std::istringstream	requestField;
	std::string			requestStatus;
	int					dataSocketFd;

	while (true)
	{
		dataSocketFd = acceptClient();
		requestField = readData(dataSocketFd);
		displayClientData(dataSocketFd);
		processCommand(requestField, dataSocketFd);
	}
}
