#include "UserMap.hpp"

void UserMap::addUser(int socket, std::string &nickname)
{
    socket_to_nickname[socket] = nickname;
    nickname_to_user[nickname] = User(nickname);
}

std::string &UserMap::getNicknameFromSocket(int socket)
{
    return socket_to_nickname[socket];
}

User& UserMap::operator[](int socket)
{
    return nickname_to_user[socket_to_nickname[socket]];
}

User& UserMap::operator[](std::string nickname)
{
    return nickname_to_user[nickname];
}

UserMap::UserMap()
{
}

UserMap::~UserMap()
{
}