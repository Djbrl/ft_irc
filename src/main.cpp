#include "IrcServer.hpp"

int main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cout << "Usage : ./ircserver <port> <password>" << std::endl;
		return 1;
	}
	// Title bar
	std::cout << CLEAR << std::endl;
	std::cout << TITLE << CLEARLINE << " Ft_irc v4.2 - " << IPADDR << " - " << PORT << RESET << std::endl;
	std::cout << BLUE << "Waiting for connections..." << RESET << std::endl << std::endl;

	IrcServer server(av[1], av[2]);
	server.run();
}
