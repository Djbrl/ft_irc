#ifndef __DEFINES_HPP__
# define __DEFINES_HPP__

//LIBRAIRES__________________________________________________________________________________________________________

# include <iostream>
# include <sstream>

# include <string>
# include <vector>
# include <map>

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

# define IPADDR         "127.0.0.1"
# define PORT           8080
# define QUEUE_BACKLOG  10
# define MAX_EVENTS     10
# define MAX_DATA_SIZE  4096

//GLOBAL_____________________________________________________________________________________________________________

typedef struct sockaddr_in	sockaddr_in_t;
extern std::vector<int>		g_clientSockets;
void						signalHandler(int signal);

#endif
