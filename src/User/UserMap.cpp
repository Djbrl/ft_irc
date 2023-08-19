#include "UserMap.hpp"

User& UserMap::operator[](int socket)
{
    return socket_to_user[socket];
}

User& UserMap::operator[](std::string nickname)
{
    return socket_to_user[nickname_to_socket[nickname]];
}

//Create a User into the map with nothing set except the socket
User *UserMap::addUser(int socket)
{
    User *user = &this->socket_to_user[socket];
    user->setSocket(socket);
    return user;
}

User *UserMap::linkUserToNickname(std::string &nickname, int socket)
{
    User *user = getUser(socket);

    nickname_to_socket.erase(user->getNickname());
    nickname_to_socket[nickname] = socket;
    return getUser(nickname);
}

//Return a *User based on a given socket, return NULL if the User doesn't exist
User *UserMap::getUser(int socket)
{
    std::map<int, User>::iterator it = this->socket_to_user.find(socket);
    if (it == this->socket_to_user.end())
        return NULL;
    return &it->second;
}

//Return a *User based on a nickname, return NULL if the User doesn't exist
User *UserMap::getUser(std::string &nickname)
{
    std::map<std::string, int>::iterator it = this->nickname_to_socket.find(nickname);
    if (it == this->nickname_to_socket.end())
        return NULL;
    User *user = getUser(it->second);
    if (user == NULL)
        throw NicknameRegisteredToEmptySocket();
    return user;
}

void UserMap::broadcastMessage(char *message)
{
    std::map<std::string, int>::iterator    it = this->nickname_to_socket.begin();
	std::string                             messagePreview(message);
	int 		                            messageLen = strlen(message);
	int 		                            dataSent = 0;
	int 		                            bytes;

    while (it != nickname_to_socket.end())
    {
        dataSent = 0;
        while (dataSent < messageLen)
        {
            if ((bytes = send(it->second, message + dataSent, messageLen - dataSent, MSG_DONTWAIT)) <= 0)
            {
                std::cerr << "Error : Couldn't send message [" + messagePreview.substr(0, messagePreview.size()/2) + "...] to client." << std::endl;
                return ;
            }
            dataSent += bytes;
        }
        it++;
    }    
}

//Remove a User by it's socket number, return false if it didn't find the User to remove, true if it found and and removed it
bool UserMap::removeUser(int socket)
{
    User *user = getUser(socket);
    if (!user)
        return (false);
    this->nickname_to_socket.erase(user->getNickname());
    return (this->socket_to_user.erase(socket));
}

bool UserMap::userExists(std::string &nickname) {

    std::map<std::string, int>::iterator it = this->nickname_to_socket.find(nickname);
    if (it != this->nickname_to_socket.end())
        return (true);
    return (false);
}

size_t   UserMap::size() const
{
    return nickname_to_socket.size();
}

User    *UserMap::updateUser(std::string &oldNick, std::string &newNick)
{
    std::map<std::string, int>::iterator it = nickname_to_socket.begin();
    int                                  socket = -1;
    while (it != nickname_to_socket.end())
    {
        if (it->first == oldNick)
        {
            socket = it->second;
            nickname_to_socket[newNick] = socket;
            socket_to_user[socket].setNickname(newNick);
        }
        it++;
    }
    if (socket != -1)
    {
        nickname_to_socket.erase(oldNick);
        return getUser(socket);
    }
    return NULL;
}

UserMap::UserMap()
{
}

UserMap::~UserMap()
{
}