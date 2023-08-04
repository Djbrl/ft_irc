#include "Channel.hpp"

Channel::Channel()
{}

Channel::~Channel()
{}

Channel::Channel(const std::string &name, User &owner) : _channelName(name), _channelOwner(owner)
{}

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
	this->_banList = cpy._banList;
	this->_modesList = cpy._modesList;
	this->_channelOwner = cpy._channelOwner;
	this->_isPrivate = cpy._isPrivate;
	return *this;
}

//METHODS_____________________________________________________________________________________________________

void Channel::addMember(User& target) {
	_membersList.push_back(target);
}


void Channel::removeMember(User& target) {
    for (std::vector<User>::iterator it = _membersList.begin(); it != _membersList.end(); ++it) {
        if (*it == target) {
            _membersList.erase(it);
            break; // Exit the loop after erasing the element
        }
    }
}

void Channel::banMember(User& target) {
	_banList.push_back(target.getNickname());
}

void Channel::unbanMember(User& target) {
    for (std::vector<std::string>::iterator it = _banList.begin(); it != _banList.end(); ++it) {
        if (*it == target.getNickname()) {
            _banList.erase(it);
            break; // Exit the loop after erasing the element
        }
    }
}

void Channel::addMode(const std::string& mode) {
	_modesList.push_back(mode);
}

void Channel::removeMode(const std::string& mode) {
    for (std::vector<std::string>::iterator it = _modesList.begin(); it != _modesList.end(); ++it) {
        if (*it == mode) {
            _modesList.erase(it);
            break; // Exit the loop after erasing the element
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
            break; // Exit the loop after erasing the element
        }
    }
}

//GETTERS_____________________________________________________________________________________________________

const std::string &Channel::getChannelName() const {
	return _channelName;
}

const std::string &Channel::getChannelTopic() const {
	return _channelTopic;
}

const std::string &Channel::getChannelPassword() const {
	return _channelPassword;
}

const User &Channel::getChannelOwner() const {
	return _channelOwner;
}

const bool &Channel::getPrivacyStatus() const {
	return _isPrivate;
}

const std::vector<User> &Channel::getMembersList() const {
	return _membersList;
}

const std::vector<User> &Channel::getOperatorsList() const {
	return _operatorsList;
}

const std::vector<std::string> &Channel::getBanList() const {
	return _banList;
}

const std::vector<std::string> &Channel::getModesList() const {
	return _modesList;
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

void Channel::setChannelOwner(User &owner) {
	_channelOwner = owner;
}

void Channel::setPrivacyStatus(bool &status) {
	_isPrivate = status;
}

//EXTERN OPERATORS_____________________________________________________________________________________________

std::ostream    &operator<<(std::ostream &flux, const Channel& rhs)
{
	flux << "Channel Name: " << rhs.getChannelName() << std::endl;
	flux << "Channel Topic: " << rhs.getChannelTopic() << std::endl;
	flux << "Channel Password: " << rhs.getChannelPassword() << std::endl;
	flux << "Channel Owner: " << rhs.getChannelOwner().getNickname() << std::endl;
	flux << "Is Private: " << (rhs.getPrivacyStatus() ? "Yes" : "No") << std::endl;

	flux << "Members List: ";
	for (size_t i = 0; i < rhs.getMembersList().size(); ++i)
	{
		flux << rhs.getMembersList()[i].getNickname();
		if (i < rhs.getMembersList().size() - 1)
			flux << ", ";
	}
	flux << std::endl;

	flux << "Ban List: ";
	for (size_t i = 0; i < rhs.getBanList().size(); ++i)
	{
		flux << rhs.getBanList()[i];
		if (i < rhs.getBanList().size() - 1)
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
	return flux;
}
