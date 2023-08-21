#ifndef __USER_MAP_HPP__
# define __USER_MAP_HPP__

# include "_defines.hpp"
# include "User.hpp"

class UserMap
{
private:
    std::map<std::string, int> nickname_to_socket;
    std::map<int, User> socket_to_user;
public:
    std::string &getNicknameFromSocket(int socket);

    User& operator[](int socket);
    User& operator[](std::string nickname);

    User *addUser(int socket); //Return the created User
    User *linkUserToNickname(std::string &nickname, int socket);
    User *getUser(int socket);
    User *getUser(std::string &nickname);
    void broadcastMessage(char *message);
    bool removeUser(int socket);
    bool userExists(std::string &nickname);
    size_t   size() const;
    User    *updateUser(std::string &oldNick, std::string &newNick);
    UserMap();
    ~UserMap();

    class NicknameRegisteredToEmptySocket : public std::exception
    {
		public:
			virtual const char* what() const throw(){
				return "IrcServer: UserMap: Nickname is registered to a socket but socket isn't registered to a User.";
			}
    };
};

#endif