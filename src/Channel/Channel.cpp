#include "Channel.hpp"

Channel::Channel()
{}

Channel::~Channel()
{}

Channel::Channel(const std::string &name, User &owner) : _channelName(name), _channelOwner(owner)
{
	this->_operatorsList.push_back(owner); //add the creator of the channel to the operators list
	this->_membersList.push_back(owner);
	this->_nbUsersLimit = 0; //no limit set
}

Channel::Channel(const Channel &cpy)
{
	if (this != &cpy)
		*this = cpy;
}

Channel& Channel::operator=(const Channel &cpy)
{
	if (this == &cpy)
		return *this;
	this->_channelName = cpy._channelName;
	this->_channelTopic = cpy._channelTopic;
	this->_channelPassword = cpy._channelPassword;
	this->_membersList = cpy._membersList;
	this->_operatorsList = cpy._operatorsList;
	this->_messageHistory = cpy._messageHistory;
	this->_modesList = cpy._modesList;
	this->_channelOwner = cpy._channelOwner;
	return *this;
}

//METHODS_____________________________________________________________________________________________________

void Channel::addMember(User& target)
{
	if (hasMember(target))
		return ;
	_membersList.push_back(target);
}

void Channel::removeMember(User& target)
{
	for (std::vector<User>::iterator it = _membersList.begin(); it != _membersList.end(); ++it)
	{
		if (*it == target)
		{
			_membersList.erase(it);
			break ;
		}
	}
}

void Channel::updateMemberNickname(std::string &oldNick, User &target)
{
	if (_channelOwner.getNickname() == oldNick)
		_channelOwner = target;
	//UPDATE IN MEMBER LIST
	for (size_t i = 0; i < _membersList.size(); i++)
	{
		if (_membersList[i].getNickname() == oldNick)
		{
			_membersList[i] = target;
			return ;
		}
	}
	//UPDATE IN OPERATOR LIST
	for (size_t i = 0; i < _operatorsList.size(); i++)
	{
		if (_operatorsList[i].getNickname() == oldNick)
		{
			_operatorsList[i] = target;
			return ;
		}
	}
	return ;
}

void Channel::addMode(const std::string& mode) {
	_modesList.push_back(mode);
}

void Channel::changeMode(const std::string &currentMode, const std::string &newMode)
{
	for (std::size_t i = 0; i < this->_modesList.size(); i++)
	{
		if (this->_modesList[i] == currentMode)
		{
			this->_modesList[i] = newMode;
			return ;
		}
	}
}

void Channel::removeMode(const std::string& mode) {
	for (std::vector<std::string>::iterator it = _modesList.begin(); it != _modesList.end(); ++it) {
		if (*it == mode) {
			_modesList.erase(it);
			break ;
		}
	}
}
void Channel::addOperator(User& target) {
	_operatorsList.push_back(target);
}

void Channel::removeOperator(User& target) {
	for (std::vector<User>::iterator it = _operatorsList.begin(); it != _operatorsList.end(); ++it) {
		if (*it == target) {
			_operatorsList.erase(it);
			break ;
		}
	}
}

void Channel::sendMessageToUsers(const std::string &messageToChannel, const std::string &author)
{
	std::string message = ":" + author + " PRIVMSG " + _channelName + " :" + messageToChannel + "\r\n";

	for (size_t i = 0; i < _membersList.size(); i++)
	{
		if (_membersList[i].getNickname() != author)
		{
			int bytes = send(_membersList[i].getSocket(), message.c_str(), message.size(), MSG_DONTWAIT); 
			if (bytes <= 0)
			{
				std::cout << "Notice : User [" << _membersList[i].getNickname() << "] cannot be reached and will be disconnected from the server." << std::endl;				continue ;
			}
			usleep(50000);
		}
	}
	addMessageToHistory(message);
	return;
}

void Channel::sendNoticeToUsers(const std::string &noticeToChannel, const std::string &author)
{
	std::string noticeMessage = "NOTICE " + author + " :" + noticeToChannel + "\r\n";

	for (size_t i = 0; i < _membersList.size(); i++)
	{
		if (_membersList[i].getNickname() != author)
		{
			int bytes = send(_membersList[i].getSocket(), noticeToChannel.c_str(), noticeToChannel.size(), MSG_DONTWAIT);
			if (bytes <= 0)
			{
				std::cout << "Notice : User [" << _membersList[i].getNickname() << "] cannot be reached and will be disconnected from the server." << std::endl;				continue ;
				continue;
			}
			usleep(50000);
		}
	}
	return;
}

void Channel::addMessageToHistory(const std::string &message)
{
	_messageHistory.push_back(message);
}

void Channel::showMessageHistory(User &target)
{
	std::string	message = "";
	int			messageLen = 0;

	if (_messageHistory.size() > 10)
	{
		for (size_t i = _messageHistory.size()- 10; i < _messageHistory.size(); i++)
			message +=_messageHistory[i] + "\r\n";
	}
	else
	{
		for (size_t i = 0; i < _messageHistory.size(); i++)
			message += _messageHistory[i] + "\r\n";
	}
	messageLen = message.size();
	int bytesSent = send(target.getSocket(), message.c_str(), messageLen, MSG_DONTWAIT);        
	if (bytesSent == -1 || bytesSent < (int)message.size())
	{
		std::cout << "Notice : User [" << target.getNickname() << "] cannot be reached and will be disconnected from the server." << std::endl;
		return;
	}
}

void Channel::removeChannelPassword() {

	if (!this->_channelPassword.empty())
		this->_channelPassword.clear();
	return ;	
}

std::vector<User>::iterator	Channel::isAMember(const std::string &userName) {

	for (std::vector<User>::iterator it = _membersList.begin(); it != _membersList.end(); ++it) {
		if (it->getNickname() == userName) {	
			return (it);
		}
	}
	return (_membersList.end());
}

std::string	Channel::printMemberList() const
{
	std::string	memberList;
	for (size_t i = 0; i < _membersList.size(); i++)
	{
		memberList += _membersList[i].getNickname();
		if(i + 1 < _membersList.size())
			memberList += ",";
	}
	return memberList;
}

//BOOL_____________________________________________________________________________________________________

bool	Channel::hasMember(const User &target) const
{
	for (std::size_t i = 0; i < _membersList.size(); i++)
	{
		if (_membersList[i] == target)
			return (true);
	}
	return (false);
}

bool	Channel::isChannelOp(User &target) const
{
	for (std::size_t i = 0; i < _operatorsList.size(); i++)
	{
		if (_operatorsList[i] == target)
			return (true);
	}
	return (false);
}

bool	Channel::findMode(std::string mode) const
{
	for (std::size_t i = 0; i < _modesList.size(); i++)
	{
		if (_modesList[i] == mode)
			return (true);
	}
	return (false);
}


//GETTERS_____________________________________________________________________________________________________

const std::string &Channel::getChannelName() const {
	return _channelName;
}

const std::string &Channel::getChannelPassword() const {
	return _channelPassword;
}

const std::string &Channel::getChannelTopic() const {
	return _channelTopic;
}

const User &Channel::getChannelOwner() const {
	return _channelOwner;
}

const std::vector<User> &Channel::getMembersList() const {
	return _membersList;
}

const std::vector<User> &Channel::getOperatorsList() const {
	return _operatorsList;
}

const std::vector<std::string> &Channel::getModesList() const {
	return _modesList;
}

const std::vector<std::string> &Channel::getMessageHistory() const
{
	return _messageHistory;
}

const std::size_t	&Channel::getNbUserLimits() const
{
	return _nbUsersLimit;
}

//SETTERS_____________________________________________________________________________________________________

void	Channel::setChannelOwner(User &newOwner) {
	_channelOwner = newOwner;
}

void	Channel::setChannelName(const std::string &name) {
	_channelName = name;
}

void	Channel::setChannelTopic(const std::string &topic) {
	_channelTopic = topic;
}

void	Channel ::setChannelPassword(const std::string &password) {
	_channelPassword = password;
}

void	Channel::setNbUserLimits(std::size_t limit) {

	_nbUsersLimit = limit;
}


//EXTERN OPERATORS_____________________________________________________________________________________________

std::ostream    &operator<<(std::ostream &flux, const Channel& rhs)
{
	flux << "Channel Name: " << rhs.getChannelName() << std::endl;
	flux << "Channel Topic: " << rhs.getChannelTopic() << std::endl;
	flux << "Channel Owner: " << rhs.getChannelOwner().getNickname() << std::endl;

	flux << "Members List: ";
	for (size_t i = 0; i < rhs.getMembersList().size(); ++i)
	{
		flux << rhs.getMembersList()[i].getNickname() << ":" << rhs.getMembersList()[i].getSocket();
		if (i < rhs.getMembersList().size() - 1)
			flux << ", ";
	}
	flux << std::endl;

	flux << "Modes List: ";
	for (size_t i = 0; i < rhs.getModesList().size(); ++i)
	{
		flux << rhs.getModesList()[i];
		if (i < rhs.getModesList().size() - 1)
			flux << ", ";
	}
	flux << std::endl;

	flux << "Message History: ";
	for (size_t i = 0; i < rhs.getMessageHistory().size(); ++i)
	{
		flux << rhs.getMessageHistory()[i];
		if (i < rhs.getMessageHistory().size() - 1)
			flux << ", ";
	}
	flux << std::endl;
	return flux;
}
