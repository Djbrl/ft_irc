#include "User.hpp"

User::User()
{
	_nickname = "";
	_username = "";
	_hostname = "";
	_servername = "";
	_realname = "";
	_lastActiveTime = time(NULL);
	_hasPassword = false;
	_socket = -1;
	memset(buffer, '\0', sizeof(buffer));
}

User::User(const std::string &name)
{
	_nickname = name;
	_username = "";
	_hostname = "";
	_servername = "";
	_realname = "";
	_lastActiveTime = time(NULL);
	_hasPassword = false;
	_socket = -1;
	memset(buffer, '\0', sizeof(buffer));
}

User::~User()
{}

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
		this->_username = cpy._realname;
		this->_username = cpy._hostname;
		this->_username = cpy._servername;
		this->_socket = cpy._socket;
		this->_channelsInvitedTo = cpy._channelsInvitedTo;
		this->_lastActiveTime = cpy._lastActiveTime;
		this->_hasPassword = cpy._hasPassword;
		memcpy(this->buffer, cpy.buffer, sizeof(cpy.buffer));
	}
	return *this;
}

//METHODS______________________________________________________________________________________________________

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
		this->_nickname != ""
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

std::string					User::getRealname() const
{
	return _realname;
}

time_t						User::getLastActiveTime() const
{
	return _lastActiveTime;
}

int							User::getSocket() const
{
	return _socket;
}

//SETTERS______________________________________________________________________________________________________

void User::setUserInfo(const std::string &uname, const std::string &hname, const std::string &servername, const std::string &realname)
{
	_username = uname;
	_hostname = hname;
	_servername = servername;
	_realname = realname;
}

void User::setNickname(const std::string &name)
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

void	User::setHasPassword(const bool status)
{
	this->_hasPassword = status;
}

void	User::setChannelsList(const std::string &channelName)
{
	for (std::size_t i = 0; i < this->_channelsInvitedTo.size(); i++)
	{
		if (this->_channelsInvitedTo[i] == channelName)
			return ; //channel is already in the vector
	}
	this->_channelsInvitedTo.push_back(channelName);
}

//BOOLEAN__________________________________________________________________________________________________

bool	User::channelIsInList(const std::string &name)
{
	for (size_t i = 0; i < this->_channelsInvitedTo.size(); i++)
	{
		if (this->_channelsInvitedTo[i] == name)
			return (true);
	}
	return (false);
}

//EXTERN OPERATORS_____________________________________________________________________________________________


std::ostream	&operator<<(std::ostream &flux, const User& rhs)
{
	flux << "User nickname: " << rhs.getNickname() << std::endl;
	flux << "User username: " << rhs.getUsername() << std::endl;
	flux << "User socket: " << rhs.getSocket() << std::endl;
	time_t time = rhs.getLastActiveTime();
	flux << "User last active time: " << std::ctime(&time);
	flux << std::endl;
	return flux;
}

bool			User::operator==(const User& other) const
{
	return _nickname == other._nickname;
}
