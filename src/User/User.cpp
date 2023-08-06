#include "User.hpp"

User::User()
{
	_nickname = "";
	_username = "";
	_registrationDate = time(NULL);
	_lastActiveTime = time(NULL);
	_isOperator = false;
	_hasPassword = false;
}

User::~User()
{}

// User::User(const std::string &name)
// {
// 	_nickname = name;
// 	_username = "";
// 	_registrationDate = time(NULL);
// 	_lastActiveTime = time(NULL);
// 	_isConnected = false;
// 	_isOperator = false;
// }

// User::User(const std::string &name, const std::string &uname)
// {
// 	_nickname = name;
// 	_username = uname;
// 	_registrationDate = time(NULL);
// 	_lastActiveTime = time(NULL);
// 	_isOperator = false;
// }

User::User(const User &cpy)
{
	if (this != &cpy)
		*this = cpy;
}

User&	User::operator=(const User &cpy)
{
	if (this != &cpy)
	{
		this->_nickname = cpy._nickname;
		this->_username = cpy._username;
		this->_registrationDate = cpy._registrationDate;
		this->_lastActiveTime = cpy._lastActiveTime;
		this->_isOperator = cpy._isOperator;
		this->_messageQueue = cpy._messageQueue;
	}
	return *this;
}

//METHODS______________________________________________________________________________________________________

void User::addMessageToQueue(const std::string& message)
{
	try
	{
		if (_messageQueue.size() >= 30 || !Utils::isPrintableStr(message))
		{
			throw MessageQueueFullException();
			return ;
		}
		/* code */
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	_messageQueue.push_back(message);
}

void User::removeMessageFromQueue(const std::string& message)
{
    if (_messageQueue.empty())
        return;

    std::vector<std::string>::iterator it = _messageQueue.begin();
    while (it != _messageQueue.end())
    {
        if (*it == message)
        {
            it = _messageQueue.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

bool User::hasPassword() const
{
	return this->_hasPassword;
}

//return true when a client has received PASS, NICK, USER
bool User::isAuthentificated() const
{
	return
	(
		this->_hasPassword &&
		!this->_username.empty() &&
		!this->_username.empty() &&
		!this->_realname.empty() &&
		!this->_hostname.empty()
	);
}

//GETTERS______________________________________________________________________________________________________

std::string					User::getNickname() const
{
	return _nickname;
}

std::string					User::getUsername() const
{
	return _username;
}

time_t						User::getRegistrationDate() const
{
	return _registrationDate;
}

time_t						User::getLastActiveTime() const
{
	return _lastActiveTime;
}

bool						User::getIsOperator() const
{
	return _isOperator;
}

std::vector<std::string>	User::getMessageQueue() const
{
	return _messageQueue;
}

int							User::getSocket() const
{
	return _socket;
}

//SETTERS______________________________________________________________________________________________________

void    User::setOperatorStatus(bool status)
{
	_isOperator = status;
}

void    User::setNickname(const std::string &name)
{
	_nickname = name;
}

void    User::setUsername(const std::string &uname)
{
	_username = uname;
}

void	User::setSocket(const int socket_fd)
{
	_socket = socket_fd;
}


//EXTERN OPERATORS_____________________________________________________________________________________________


std::ostream	&operator<<(std::ostream &flux, const User& rhs)
{
	time_t time = rhs.getRegistrationDate();
	flux << "User nickname: " << rhs.getNickname() << std::endl;
	flux << "User username: " << rhs.getUsername() << std::endl;
	flux << "User registration date: " << std::ctime(&time);
	time = rhs.getLastActiveTime();
	flux << "User last active time: " << std::ctime(&time);
	flux << "User messages in queue: \n";
	std::cout << "[";
	for (size_t i = 0; i < rhs.getMessageQueue().size(); ++i)
	{
		flux << "[" << rhs.getMessageQueue()[i] << "]";
		if (i < rhs.getMessageQueue().size() - 1)
			flux << ", ";
	}
	std::cout << "]";
	flux << std::endl;
	return flux;
}

bool			User::operator==(const User& other) const
{
		return _nickname == other._nickname;
}
