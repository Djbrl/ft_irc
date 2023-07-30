#ifndef __CHANNEL_HPP__
# define __CHANNEL_HPP__

# include "_defines.hpp"
# include "User.hpp"

//CHANNEL CLASS____________________________________________________________________________________________________
//Channel is a concrete class that carries all the methods and attributes needed to create channels

class Channel
{
	private:
		std::string                 _channelName;
		std::string                 _channelTopic;
		std::string                 _channelPassword;
		std::vector<User>           _membersList;
		std::vector<User>           _operatorsList;
		std::vector<std::string>    _banList;
		std::vector<std::string>    _modesList;
		User                        _channelOwner;
		bool                        _isPrivate;

	public:
									Channel(const std::string &name, User &owner);
									Channel();
									~Channel();
									Channel(const Channel &cpy);
	Channel							&operator=(const Channel &cpy);
    
	//METHODS__________________________________________________________________________________________________
	
	void							addMember(User &target);
	void							removeMember(User &target);
	void							banMember(User &target);
	void							unbanMember(User &target);
	void							addMode(const std::string &mode);
	void							removeMode(const std::string &mode);
	void							addOperator(User &target);
	void							removeOperator(User &target);
    
	//GETTERS__________________________________________________________________________________________________
    
	const std::string				&getChannelName() const;
    const std::string				&getChannelTopic() const;
    const std::string				&getChannelPassword() const;
    const User						&getChannelOwner() const;
    const bool						&getPrivacyStatus() const;
	const std::vector<User>			&getMembersList() const;
	const std::vector<User>			&getOperatorsList() const;
	const std::vector<std::string>	&getBanList() const;
	const std::vector<std::string>	&getModesList() const;

    //SETTERS__________________________________________________________________________________________________
    void							setChannelName(const std::string &name);
    void							setChannelTopic(const std::string &topic);
    void							setChannelPassword(const std::string &password);
    void							setChannelOwner(User &owner);
    void							setPrivacyStatus(bool &status);
};

//EXTERN OPERATORS_____________________________________________________________________________________________
std::ostream               &operator<<(std::ostream &flux, const Channel& src);

#endif
