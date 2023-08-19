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
		std::string					_realname;
		std::string					_hostname;
		std::string					_servername;
		std::vector<std::string>	_channelsInvitedTo; //All the channels the user has been invited to
		time_t						_lastActiveTime;
		bool						_hasPassword;
		int							_socket;

	public:
		char						buffer[MESSAGE_BUFFER_SIZE];
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
	std::string						getRealname() const;
    time_t							getLastActiveTime() const;
	int								getSocket() const;
    
	//SETTERS__________________________________________________________________________________________________
	void							setUserInfo(const std::string &uname, const std::string &hname, const std::string &servername, const std::string &realname);
	void							setNickname(const std::string &name);
	void							setUsername(const std::string &uname);
	void							setHasPassword(const bool state);
	void							setSocket(const int socket_fd);
	void							setChannelsList(const std::string &channelName);

	//BOOLEAN__________________________________________________________________________________________________

	bool							channelIsInList(const std::string &name);

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
