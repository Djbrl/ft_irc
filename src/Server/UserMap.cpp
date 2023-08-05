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

//Remove a User by it's socket number, return false if it didn't find the User to remove, true if it found and and removed it
bool UserMap::removeUser(int socket)
{
    User *user = getUser(socket);
    if (!user)
        return false;
    this->nickname_to_socket.erase(user->getNickname());
    return (this->socket_to_user.erase(socket));
}

UserMap::UserMap()
{
}

UserMap::~UserMap()
{
}