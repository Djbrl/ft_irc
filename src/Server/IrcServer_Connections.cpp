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

std::string     IrcServer::parsePassCommand(int clientFd, std::stringstream &commandCopy) {

    std::vector<std::string>    wordSplit;
    std::string                 word;

    (void)clientFd;

    while(commandCopy >> word)
        wordSplit.push_back(word);
    //printing vector to test
    // for (std::size_t i = 0; i < wordSplit.size(); i++)
    //     std::cout << wordSplit[i] << std::endl;
    if (wordSplit.size() == 0)
        std::cout << "Password missing !" << std::endl;
    for (std::size_t i = 0; i < wordSplit.size(); i++)
    {
        if ((Utils::isEven(i) && wordSplit[i] == PASS) || (Utils::isOdd(i) && wordSplit[i] != PASS))
        {
            std::cerr << "Error : SOMETHING WRONG WITH PASSWORD." << std::endl;
            return ("");
        }
    }
    return (word);
}

void    IrcServer::passCommand(int clientFd, std::string passCommand, std::stringstream &commandCopy)
{
        std::string clientPassword;
        (void)clientFd;
        
        //if the client is NOT already connected {

            if (passCommand.find(USER) != std::string::npos || passCommand.find(NICK) != std::string::npos
                || passCommand.find(INVITE) != std::string::npos || passCommand.find(MODE) != std::string::npos 
                || passCommand.find(TOPIC) != std::string::npos || passCommand.find(KICK) != std::string::npos)
            {
                std::cerr << "Error : There cannot be different commands." << std::endl;
                return ;
            }
            if (!(clientPassword = parsePassCommand(clientFd, commandCopy)).empty())
            {
                if (clientPassword == this->_serverPassword)
                {
                    //add user to connected users
                    //set user authenticated status to true
                    std::cout << "PASSWORD OK !" << std::endl;
                }
                else
                {
                    std::cerr << "Error : SOMETHING WRONG WITH PASSWORD." << std::endl;
                    return ;
                }
            }

        // }
        //else
        // {
            //Tell client he is already authentified ?
        // }


}

void    IrcServer::queryDispatch(int clientFd, std::string clientQuery) {

    std::stringstream   queryCopy(clientQuery);
    std::string         command;
    (void)clientFd;
    
    queryCopy >> command;

    // if(client is not authentified && command.compare(PASS) != 0)
        //tell him that authentification is needed
    if (command.compare(PASS) == 0)
        passCommand(clientFd, clientQuery, queryCopy);
    // else if (command.compare(USER) == 0)
    //     userCommand(queryCopy);
    // else if (command.compare(NICK) == 0)
    //     nickCommand(queryCopy);
    // else if (command.compare(INVITE) == 0)
    //     inviteCommand(queryCopy);
    // else if (command.compare(MODE) == 0)
    //     modeCommand(queryCopy);
    // else if (command.compare(TOPIC) == 0)
    //     topicCommand(queryCopy);
    // else if (command.compare(KICK) == 0)
    //     kickCommand(queryCopy);
    // else
    // {
    //     handle whatever we have to handle if it is none of the above !
    // }

}

void	IrcServer::parseQuery(int clientFd, std::string clientQuery) {

    std::size_t queryLen = clientQuery.size();
    std::size_t start = 0;
    std::size_t end, found;
    std::string subQuery;

    //check basics
    if (queryLen > 512)
        std::cerr << " Error : Client's request is too long. " << std::endl;
    if (clientQuery[queryLen - 2] != '\r' || clientQuery[queryLen - 1] != '\n')
        std::cerr << "Error : Client's request does not end with the appropriate characters." << std::endl;
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

        //implement the parsing logic for each command individually but dispatch them first
        queryDispatch(clientFd, subQuery);

        //update start to analyze the rest of the query
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
    // std::cout << Utils::getLocalTime() << "[" << Utils::trimBackline(socketData) << "] received from client[" << clientSocket << "] " << BYELLOW << clientIP << RESET << "." << std::endl;
    std::cout << "Socket is " << clientSocket << std::endl;
    parseQuery(clientSocket, socketData);
    return;
}