#include "IrcServer.hpp"

int main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cout << "Usage : ./ircserver" << BWHITE << " <port> <password>\n" << RESET << std::endl;
		return 1;
	}

	unsigned int	serverPort(atoi(av[1]));
	std::string		serverPassword(av[2]);
	if ((serverPort == 0 && av[2] != std::string("0")) || serverPassword.empty())
	{
		std::cout << "Error: Invalid port or empty password." << std::endl;
		return 1;
	}
	std::cout << CLEAR << std::endl;
	std::cout << TITLE << CLEARLINE << " Ft_irc v4.2 - localhost" << " - " << serverPort << RESET << std::endl;
	std::cout << BLUE << "Waiting for connections..." << RESET << std::endl << std::endl;
	IrcServer server(serverPort, serverPassword);
	server.run();
}
