#include "IrcServer.hpp"

void    IrcServer::decorticateMessage(int targeted_client, char *msg)
{
    int sent;
	int already_sent = 0;
	int msg_len = strlen(msg); //length of the message to be sent

	while (already_sent < msg_len)
    {
		if ((sent = send(targeted_client, msg + already_sent, msg_len - already_sent, MSG_DONTWAIT)) <= 0)
			return ;
		already_sent += sent;
	}  
}

//sending the message to every client that is not the sender !
void    IrcServer::sendMessage(int sender_fd, char *msg)
{
    for (unsigned int i = 0; i < g_clientSockets.size(); i++)
    {
        if (g_clientSockets[i] != sender_fd)  
            decorticateMessage(g_clientSockets[i], msg);
    }

}

// Accept connections from clients on the serverFd and print a message with the clientFD and the IP
int IrcServer::acceptClient()
{
    sockaddr	clientSockAddr;
	std::string	clientIP;
    int			clientAddrLen;
    int			dataSocketFd;

	try
    {
        clientAddrLen = sizeof(_serverSockAddr);
        if ((dataSocketFd = accept(_serverFd, (struct sockaddr *)&clientSockAddr, (socklen_t *)&clientAddrLen)) == -1)
            throw AcceptException();

        FD_SET(dataSocketFd, &_clientsFdSet);
        g_clientSockets.push_back(dataSocketFd);
    	clientIP = inet_ntoa(((struct sockaddr_in *)&clientSockAddr)->sin_addr);
        std::cout << Utils::getLocalTime() << "New client connection : [" << dataSocketFd << "] - " << BYELLOW << clientIP << RESET << "." << std::endl;
        // SEND BACK NUMERIC REPLY TO CLIENT //
        /* send_message(new_sockfd, welcome_msg); */
        // SEND BACK NUMERIC REPLY TO CLIENT //
    }
    catch (const AcceptException &e)
    {
        std::cerr << e.what() << '\n';
    }
    return dataSocketFd;
}

void	IrcServer::parseQuery(std::string clientQuery) {

    // std::size_t queryLen = clientQuery.size(); //check that it is not over 512
    std::size_t start = 0;
    std::size_t end, found;
    std::string subQuery;

    //check basics

    // if (_queryLen > 512)
        //throw something 

    

    //ignore spaces
    while (clientQuery[start] == ' ')
        start++;

    found = clientQuery.find("\r\n");
    
    while(found != std::string::npos)
    {
        end = found;
        //ignore spaces
        if (clientQuery[found - 1] == ' ')
        {
            end = found;
            while (clientQuery[end - 1] == ' ')
                end--;
        }
        //extract each command until the next "\r\n"
        subQuery = clientQuery.substr(start, end - start);
        std::cout << "Substring : " << " [" <<subQuery << "]" << std::endl;
        
        //implement the parsing logic for each command individually
        
        //update _start to analyze the rest of the query
        start = found + 2;
        //ignore spaces
        while(clientQuery[start] == ' ')
            start++;
        //find the next "\r\n" occurrence
        found = clientQuery.find("\r\n", start);
    }

}

//Display the data received from the current connected client
void IrcServer::printSocketData(int clientSocket, char* socketData)
{
    std::string	clientIP;

	clientIP = inet_ntoa(_serverSockAddr.sin_addr);
    std::cout << Utils::getLocalTime() << "[" << Utils::trimBackline(socketData) << "] received from client[" << clientSocket << "] " << BYELLOW << clientIP << RESET << "." << std::endl;
    // std::cout << "Socket is " << clientSocket << std::endl;
    // parseQuery(socketData);
    return;
}