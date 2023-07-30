#ifndef __USER_HPP__
# define __USER_HPP__

# include "_defines.hpp"

//USER CLASS____________________________________________________________________________________________________
//User is a concrete class that carries methods and attributes for all clients (operators and regular users).

class User
{
	private:
		std::vector<std::string>	_messageQueue;
		std::string					_nickname;
		std::string					_username;
		time_t						_registrationDate;
		time_t						_lastActiveTime;
		bool						_isConnected;
		bool						_isOperator;

	public:
									User();
									User(const std::string &name);
									User(const std::string &name, const std::string &uname);
									~User();
									User(const User &cpy);
	User							&operator=(const User &cpy);
    
	//METHODS__________________________________________________________________________________________________
    
	void 							addMessageToQueue(const std::string& message);
    void 							removeMessageFromQueue(const std::string& message);
									//authenticate
									//joinChannel
									//sendMessage
									//sendPrivateMessage
    //OPERATOR-ONLY____________________________________________________________________________________________
	
									//kickUser
									//inviteUser
									//viewChannelTopic
									//setChannelTopic
									//setChannelMode
	//GETTERS__________________________________________________________________________________________________
	
	std::string						getNickname() const;
	std::string						getUsername() const;
    time_t							getRegistrationDate() const;
    time_t							getLastActiveTime() const;
	bool							getIsConnected() const;
	bool							getIsOperator() const;
    std::vector<std::string>		getMessageQueue() const;
    
	//SETTERS__________________________________________________________________________________________________
	
	void							setConnectedStatus(bool status);
	void							setOperatorStatus(bool status);
	void							setNickname(const std::string &name);
	void							setUsername(const std::string &uname);

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
