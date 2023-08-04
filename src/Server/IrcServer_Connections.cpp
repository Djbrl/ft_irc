#include "IrcServer.hpp"

//Accept connections from clients on the serverFd and print a message with the clientFD and the IP
int IrcServer::acceptClient()
{
	sockaddr	clientSockAddr;
	int			clientAddrLen;
	int     	dataSocketFd;
	char    	clientIP[INET_ADDRSTRLEN];

	try
	{
		clientAddrLen = sizeof(_serverSockAddr);
		if ((dataSocketFd = accept(_serverFd, (struct sockaddr *)&clientSockAddr, (socklen_t *)&clientAddrLen)) == -1)
			throw AcceptException();
		FD_SET(dataSocketFd, &_clientsFdSet);
		g_clientSockets.push_back(dataSocketFd);
		inet_ntop(AF_INET, &(_serverSockAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
		std::cout << Utils::getLocalTime() << "New client connection : [" << dataSocketFd << "] - " << BYELLOW << clientIP << RESET << "." << std::endl;
		// SEND BACK NUMERIC REPLY TO CLIENT 		//
		/* send_message(new_sockfd, welcome_msg);	*/
		// SEND BACK NUMERIC REPLY TO CLIENT 		//
	}
	catch(const AcceptException &e)
	{
		std::cerr << e.what() << '\n';
	}
	return dataSocketFd;
}

//Display the data received from the current connected client
void IrcServer::printSocketData(int clientSocket, char *socketData)
{
	char    clientIP[INET_ADDRSTRLEN];

	inet_ntop(AF_INET, &(_serverSockAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
	std::cout << Utils::getLocalTime() << "[" << Utils::trimBackline(socketData) << "] received from client[" << clientSocket << "] " << BYELLOW << clientIP << RESET << "." << std::endl;
}
