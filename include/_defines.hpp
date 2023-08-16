#ifndef __DEFINES_HPP__
# define __DEFINES_HPP__

//LIBRAIRES__________________________________________________________________________________________________________

# include <iostream>
# include <sstream>
# include <cstdlib>
# include <cstdio>

# include <cstring>
# include <vector>
# include <map>
# include <algorithm>

# include <csignal>
# include <ctime>

# include <unistd.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <fcntl.h>
# include <poll.h>

# include "Utils.hpp"

//ANSI CODES_________________________________________________________________________________________________________

# define CLEARLINE  "\033[2K"
# define CLEAR      "\033[2J\033[H"
# define TITLE      "\033[44;97m"

# define BWHITE     "\033[1;37m"
# define BYELLOW    "\033[1;33m"
# define BBLUE      "\033[1;94m"

# define YELLOW     "\033[0;33m"
# define GREEN      "\033[0;32m"
# define RED        "\033[0;31m"
# define BLUE       "\033[0;34m"
# define RESET      "\033[0m"

//DEFINES____________________________________________________________________________________________________________

# define IPADDR                 "127.0.0.1"
# define HOSTNAME				"localhost"
# define VERSION				"ft_irc4.2"
# define PORT                   6667
# define MAX_EVENTS             10
# define MAX_DATA_SIZE          4096
# define MESSAGE_BUFFER_SIZE    1024

//COMMANDS___________________________________________________________________________________________________________

# define PASS "PASS"
# define NICK "NICK"
# define USER "USER"
# define JOIN "JOIN"
# define TOPIC "TOPIC"
# define INVITE "INVITE"
# define KICK "KICK"
# define MODE "MODE"

//GLOBAL_____________________________________________________________________________________________________________

typedef struct sockaddr_in	sockaddr_in_t;
extern std::vector<int>		g_clientSockets;
void						signalHandler(int signal);

//NUMERIC REPLIES____________________________________________________________________________________________________

//CONNECT
# define RPL_WELCOME(clientNickname)                                (std::string(":" HOSTNAME " 001 ") + clientNickname + " :Welcome to the FT_IRC server " + clientNickname + "!~" + clientNickname + "@" + std::string(HOSTNAME) +"! \r\n")
# define RPL_YOURHOST(clientNickname)                               (std::string(":" HOSTNAME " 002 ") + clientNickname + " :Your host is " + HOSTNAME + ", running version " VERSION "\r\n")
# define RPL_CREATED(clientNickname, date)	                        (std::string(":" HOSTNAME " 003 ") + clientNickname + " :This server was created : " + std::ctime(&date) + "\r\n")
# define RPL_MYINFO(clientNickname)                                 (std::string(":" HOSTNAME " 004 ") + clientNickname + " " + HOSTNAME + " " VERSION " . itkol\r\n")
# define RPL_PART(clientNickname, channel)							(std::string(":" HOSTNAME " 324 ") + clientNickname + " " + channel + " :You have left " + channel + "\r\n")
# define RPL_PONG(param)					                        ("PONG :" + std::string(param) + "\r\n")
# define RPL_PARTNOTICE(clientNickname, channel)					(":ft_irc 400 " + clientNickname + " " + channel + " :- You have left the channel\r\n")

//OTHER
# define RPL_TOPIC(clientNickname, channel, topic)		            (":ft_irc 332 " + clientNickname + " " + channel + " :" + topic + "\r\n")
# define RPL_NOTOPIC(clientNickname, channel)                       (std::string(":" HOSTNAME " 331 ") + clientNickname + " " + channel + " :No topic is set\r\n")
# define RPL_INVITING(clientNickname, invitedNick, channel)         (std::string(":" HOSTNAME " 341 ") + clientNickname + " " + invitedNick + " " + channel + " :Was succesfully invited\r\n")    
# define RPL_NAMREPLY(clientNickname, channel, names)	            (":" + std::string(HOSTNAME) + " 353 " + clientNickname + " = " + channel + " :" + names +"\r\n")
# define RPL_ENDOFNAMES(clientNickname, channel)		            (":" + std::string(HOSTNAME) + " 366 " + clientNickname + " " + channel + " :End of /NAMES list\r\n")
# define RPL_ALREADYREGISTRED(clientNickname, channel)	            (":" + std::string(HOSTNAME) + " 403 " + clientNickname + " " + channel + " :You are already in that channel\r\n")
# define ERR_NEEDMOREPARAMS(clientNickname, command)				(":" + std::string(HOSTNAME) + " 461 " + clientNickname + " " + command + " :Not enough parameters\r\n")
# define RPL_CHANNELMODEIS(channel, modes)                          (":" + std::string(HOSTNAME) + " 324 " + channel + " " + modes + "\r\n")

# define ERR_NOSUCHNICK(clientNickname, nickNotFound)               (":" + std::string(HOSTNAME) + " 401 " + clientNickname + " " + nickNotFound + " :No such nick\r\n")
# define ERR_NOSUCHCHANNEL(clientNickname, channel)		            (":" + std::string(HOSTNAME) + " 403 " + clientNickname + " " + channel + " :No such channel\r\n")
# define ERR_NOTONCHANNEL(clientNickname, channel)                  (":" + std::string(HOSTNAME) + " 442 " + clientNickname + " " + channel + " :You're not on that channel\r\n")
# define ERR_USERONCHANNEL(clientNickname, nickToInvite, channel)   (":" + std::string(HOSTNAME) + " 443 " + clientNickname + " " + nickToInvite + " " + channel + " :is already on channel\r\n")
# define ERR_NOTREGISTERED(clientNickname)				            (":" + std::string(HOSTNAME) + " 451 " + clientNickname + " :You have not registered\r\n")
# define ERR_PASSWDMISMATCH(clientNickname)				            (":" + std::string(HOSTNAME) + " 464 " + clientNickname + " :Password incorrect\r\n")
# define ERR_PASSACCEPTED(clientNickname)				            (":" + std::string(HOSTNAME) + " 464 " + clientNickname + " :Password already sent\r\n")
# define ERR_CHANOPRIVSNEEDED(clientNickname, channel)              (":" + std::string(HOSTNAME) + " 482 " + clientNickname + " " + channel + " :You're not channel operator\r\n")
# define ERR_USERNOTINCHANNEL(clientNickname, channel)              (":" + std::string(HOSTNAME) + " 482 " + clientNickname + " " + channel + " :User not in channel\r\n")

#endif
