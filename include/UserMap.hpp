#ifndef __USER_MAP_HPP__
# define __USER_MAP_HPP__

# include "_defines.hpp"
# include "User.hpp"

class UserMap
{
private:
    std::map<int, std::string> socket_to_nickname;
    std::map<std::string, User> nickname_to_user;
public:
    std::string &getNicknameFromSocket(int socket);
    void addUser(int socket, std::string &nickname);

    User& operator[](int socket);
    User& operator[](std::string nickname);
    
    void addUser();
    
    UserMap();
    ~UserMap();
};

#endif