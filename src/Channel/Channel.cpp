#include "Channel.hpp"

Channel::Channel()
{}

Channel::~Channel()
{}

Channel::Channel(const std::string &name, User &owner) : _channelName(name), _channelOwner(owner)
{
	this->_operatorsList.push_back(owner); //add the creator of the channel to the operators list
	this->_membersList.push_back(owner);
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

void Channel::removeMember(User& target) {
	for (std::vector<User>::iterator it = _membersList.begin(); it != _membersList.end(); ++it) {
		if (*it == target) {
			_membersList.erase(it);
			break ;
		}
	}
}

void Channel::updateMemberNickname(std::string &oldNick, User &target)
{
	if (_channelOwner.getNickname() == oldNick)
		_channelOwner = target;
	for (size_t i = 0; i < _membersList.size(); i++)
	{
		if (_membersList[i].getNickname() == oldNick)
		{
			_membersList[i] = target;
			return ;
		}
	}
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
				std::cout << "sending message to : " << _membersList[i].getNickname() << std::endl;
				int bytes = send(_membersList[i].getSocket(), message.c_str(), message.size(), MSG_DONTWAIT); 
				if (bytes <= 0)
				{
					std::cout << "Error : Couldn't send data to client." << _membersList[i].getNickname() << ":" << _membersList[i].getSocket() << std::endl;
					continue ;
				}
				usleep(50000);
		}
	}
	addMessageToHistory(message);
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

	for (int i = 0; i < (int)_messageHistory.size(); i++)
		message += _messageHistory[i] + "\r\n"; // Add line break between messages
	messageLen = message.size();
	int bytesSent = send(target.getSocket(), message.c_str(), messageLen, MSG_DONTWAIT);        
	if (bytesSent == -1 || bytesSent < (int)message.size())
	{
		std::cout << "Error: Couldn't send data to client." << target.getNickname() << ":" << target.getSocket() << std::endl;
		return;
	}
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
		memberList += _membersList[i].getNickname() + ",";
	}
	memberList.pop_back();
	return memberList;
}

//BOOL_____________________________________________________________________________________________________

bool Channel::hasMember(const User &target) const
{
	for (std::vector<User>::const_iterator it = _membersList.begin(); it != _membersList.end(); ++it)
	{
		if (*it == target)
			return (true);
	}
	return (false);
}

bool	Channel::isChannelOp(User &target) {

	for (std::size_t i = 0; i < _operatorsList.size(); i++)
	{
		if (_operatorsList[i] == target)
			return (true);
	}
	return (false);
}

//GETTERS_____________________________________________________________________________________________________

const std::string &Channel::getChannelName() const {
	return _channelName;
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

//SETTERS_____________________________________________________________________________________________________

void Channel::setChannelName(const std::string &name) {
	_channelName = name;
}

void Channel::setChannelTopic(const std::string &topic) {
	_channelTopic = topic;
}

void Channel::setChannelPassword(const std::string &password) {
	_channelPassword = password;
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
