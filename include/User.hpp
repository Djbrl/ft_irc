#ifndef __USER_HPP__
# define __USER_HPP__

# include "_defines.hpp"

//USER CLASS____________________________________________________________________________________________________
//User is a concrete class that carries methods and attributes for all clients (operators and regular users).

class User
{
	private:
		std::string					_nickname;
		std::string					_username;
		time_t						_lastActiveTime;
		bool						_hasPassword;
		int							_socket;

	public:
									User();
									User(const std::string &name);
									~User();
									User(const User &cpy);
	User							&operator=(const User &cpy);
    
	//METHODS__________________________________________________________________________________________________
    
	bool							isAuthentificated() const;
	bool							hasPassword() const;

    //OPERATOR-ONLY____________________________________________________________________________________________
	
	//GETTERS__________________________________________________________________________________________________
	
	std::string						getNickname() const;
	std::string						getUsername() const;
    time_t							getLastActiveTime() const;
	int								getSocket() const;
    
	//SETTERS__________________________________________________________________________________________________
	
	void							setNickname(const std::string &name);
	void							setUsername(const std::string &uname);
	void							setHasPassword(const bool state);
	void							setSocket(const int socket_fd);

    //OPERATORS________________________________________________________________________________________________
	//Silence error with std::remove, _nickname is the identifier to compare two users
	bool							operator==(const User& other) const;

	//EXCEPTIONS_______________________________________________________________________________________________
	class MessageQueueFullException : public std::exception
    {
		public:
			virtual const char* what() const throw(){
				return "IrcServer: UserException: Message queue is full.";
			}
    };
};

//EXTERN OPERATORS_____________________________________________________________________________________________
std::ostream						&operator<<(std::ostream &flux, const User& rhs);

#endif
