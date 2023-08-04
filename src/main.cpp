#include "IrcServer.hpp"

int main(int ac, char **av)
{
	unsigned int	serverPort = atoi(av[1]);
	std::string		serverPassword(av[2]);

	if (ac != 3)
	{
		std::cout << "Usage : ./ircserver <port> <password>" << std::endl;
		return 1;
	}
	if ((serverPort == 0 && av[2] != "0") || serverPort < 0 || serverPassword.empty())
	{
		std::cout << "Error: Invalid port or empty password." << std::endl;
		return 1;
	}

	// Title bar
	std::cout << CLEAR << std::endl;
	std::cout << TITLE << CLEARLINE << " Ft_irc v4.2 - " << IPADDR << " - " << PORT << RESET << std::endl;
	std::cout << BLUE << "Waiting for connections..." << RESET << std::endl << std::endl;
	IrcServer server(serverPort, av[2]);
	server.run();
}
