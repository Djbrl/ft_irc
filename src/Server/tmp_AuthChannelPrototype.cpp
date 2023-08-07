#include "IrcServer.hpp"

void	IrcServer::dsy_cbarbit_AuthAndChannelMethodsPrototype(int clientFd, char *buffer)
{
    //REFACTOR INTO METHODS
    std::stringstream request(buffer);
    std::string command;
    std::string argument;
    std::string argument2;

    request >> command;
    request >> argument;

    User *user = _ConnectedUsers.getUser(clientFd);
    //COMMAND WITHOUT PASS
    if (command != "PASS" && !argument.empty() && !user->hasPassword())
    {
            std::string message = "Please enter the server password first.\r\n";
            safeSendMessage(clientFd, const_cast<char*>(message.c_str()));
            return ;
    }

    //PASS COMMAND PROTOTYPE
    //IF the command is PASS, and it has an argument, and the client hasn't logged in yet
    if (command == "PASS" && !argument.empty() && !user->hasPassword())
    {
        if (argument == _serverPassword)
        {
            user->setHasPassword(true);
            std::string message = "Set your NICK <nickname> to start using the server.\r\n";
            safeSendMessage(clientFd, const_cast<char*>(message.c_str()));
        }
        else
        {
            std::string message = "Wrong password.\r\n";
            safeSendMessage(clientFd, const_cast<char*>(message.c_str()));
            return ;
        }
    }
    else 	//IF the client is already logged in
    {
        if (command == "PASS" && !argument.empty() && user->hasPassword())
        {
            std::string message = "You're already logged in.\r\n";
            safeSendMessage(clientFd, const_cast<char*>(message.c_str()));
            return ;
        }
    }

    //NICK COMMAND PROTOTYPE
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
        else		//IF the user is not known, check if it already has a nickname (update), otherwise set it
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
                user->setNickname(argument);
                std::string message = "Hello " + user->getNickname() + "! You now have user-access to the server.\r\n";
                safeSendMessage(clientFd, const_cast<char*>(message.c_str()));
            }
        }
    }

    //JOIN COMMAND PROTOTYPE
    //ADD reading message history when user has joinned channel
    if (command == "JOIN" && !argument.empty() && user->isAuthentificated())
    {
        if (argument.size() > 1 && argument.substr(0, 1) == "#")
        {
            std::string channelName = argument.substr(1, argument.size());
            std::string message = "[" + argument + "]\r\n";
            addChannel((channelName), *user);
            _Channels[channelName].addMember(*user);
            safeSendMessage(clientFd, const_cast<char *>(message.c_str()));
        }
        else
        {
            std::string message = "Invalid channel name.\r\n";
            safeSendMessage(clientFd, const_cast<char*>(message.c_str()));
            return ;
        }
    }

    //PRIVMSG COMMAND PROTOTYPE
    std::string msg;
    request >> msg;
    if (command == "PRIVMSG" && !argument.empty() && !msg.empty() && user->isAuthentificated())
    {
        if (argument.size() > 1 && argument.substr(0, 1) == "#")
        {
            std::string channelName = argument.substr(1, argument.size());
            _Channels[channelName].sendMessageToUsers(msg, user->getNickname());
        }
        else
        {
            std::string message = "Invalid PRIVMSG args.\r\n";
            safeSendMessage(clientFd, const_cast<char*>(message.c_str()));
            return ;
        }
    }

    //KICK COMMAND PROTOTYPE
    std::stringstream myrequest(buffer);
    // std::string cmd;
    // std::string arg1;
    // std::string arg2;

    // myrequest >> cmd;
    // myrequest >> arg1;
    // myrequest >> arg2;
    if (command == "KICK" && user->isAuthentificated())
    {
        //IMPOSSIBLE DE PRINT CORRECTEMENT LE BUFFER
        std::cout << buffer << std::endl;
    }
}