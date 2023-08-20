#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>

#define CLEARLINE  "\033[2K"
#define CLEAR      "\033[2J\033[H"
#define TITLE      "\033[44;97m"

#define BWHITE     "\033[1;37m"
#define BYELLOW    "\033[1;33m"
#define BBLUE      "\033[1;94m"

#define YELLOW     "\033[0;33m"
#define GREEN      "\033[0;32m"
#define RED        "\033[0;31m"
#define BLUE       "\033[0;34m"
#define RESET      "\033[0m"

const std::string SERVER_ADDRESS = "127.0.0.1";
const int SERVER_PORT = 6666;

int initClient(char **av, int serverPort, std::string &password)
{
    sockaddr_in serverAddress;
    char SERVER_RES[1024] = {0};
    int clientSocket;

    // CONNECT
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
        std::cerr << "Error creating socket" << std::endl;
        return -1;
    }
    inet_pton(AF_INET, SERVER_ADDRESS.c_str(), &serverAddress.sin_addr);
    if (connect(clientSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1)
    {
        std::cerr << "Error connecting to server" << std::endl;
        close(clientSocket);
        return -1;
    }

    // FETCH WELCOME MESSAGE
    int bytes_received = recv(clientSocket, SERVER_RES, sizeof(SERVER_RES), 0);
    if (bytes_received <= 0)
    {
        std::cout << "Server connection closed or error occurred." << std::endl;
        return -1;
    }
    std::string RES(SERVER_RES);
    if (RES.substr(0, 42) != "You have successfully connected to ft_IRC!")
        std::cout << RED << "CONNECTION KO" << RESET << " for client " << clientSocket << std::endl;

    return clientSocket;
}

int main(int ac, char **av)
{
    // CHECK ARGUMENTS
    if (ac < 4)
    {
        std::cout << "Usage : /<port> <password> <nb of users>\n" << std::endl;
        return 1;
    }
    std::vector<int>	clients;
    std::string 		SERVER_PASSWORD = av[2];
    int					SERVER_PORT = atoi(av[1]);
    int					nbTest = atoi(av[3]);
    int					ko_count = 0;
    if ((nbTest < 1 || nbTest > 20) || SERVER_PORT < 1 || SERVER_PASSWORD.empty())
    {
        std::cout << "Usage : /<port> <password> <nb of users>\n" << std::endl;
        std::cout << YELLOW << "To avoid memory performance issues, you are limited to 20 clients, you main increase this number at your own discretion on line 77, or increase your call-stack depth.\n" << RESET << std::endl;
        return 1;
    }

    // INIT TESTING SESSION
    int ownerAccountSession = initClient(av, SERVER_PORT, SERVER_PASSWORD);
	if (ownerAccountSession == -1)
		return 1;
    std::string nick = "ownerAccountSessionClient";
    std::string uname = "session";
    std::string rname = "Testing Session";
    char CLIENT_REQ_SESSION[512] = {0};
    char CLIENT_REQ[2048] = {0};
	char SERVER_RES[512] = {0};
    snprintf(CLIENT_REQ_SESSION, sizeof(CLIENT_REQ_SESSION), "PASS %s\r\nNICK %s\r\nUSER %s 0 * :%s\r\nJOIN #testSession\r\n", SERVER_PASSWORD.c_str(), nick.c_str(), uname.c_str(), rname.c_str());
    send(ownerAccountSession, CLIENT_REQ_SESSION, strlen(CLIENT_REQ_SESSION), 0);

    //_________________________START OF TESTING GROUND____________________________________________________________________________________________________________________________________//
    //_________________________AUTHENTICATION_____________________________________________________________________________________________________________________________________________//
	//INIT CLIENTS
	for (int i = 0; i < nbTest ; i++)
    {
		int clientSocket = initClient(av, SERVER_PORT, SERVER_PASSWORD);
		if (clientSocket == -1)
		{
			std::cout << "Error : test clients initialization failed"  << std::endl;
			return 1;
		}

		std::string nickname = "TesterClient";
		std::string username = "test_client";
		std::string realname = "Tester Client";

		snprintf(CLIENT_REQ, sizeof(CLIENT_REQ), "PASS %s\r\nNICK %s\r\nUSER %s 0 * :%s\r\n", SERVER_PASSWORD.c_str(), nickname.c_str(), username.c_str(), realname.c_str());
		send(clientSocket, CLIENT_REQ, strlen(CLIENT_REQ), 0);

		int bytes_received = recv(clientSocket, SERVER_RES, sizeof(SERVER_RES), 0);
		if (bytes_received <= 0)
			std::cout << "Server connection closed or error occurred" << std::endl;
		std::string RES(SERVER_RES);
		if (RES.substr(0, 14) != ":localhost 001")
		{
			ko_count++;
			std::cout << RED << "AUTH TEST KO" << RESET << " for " << CLIENT_REQ << std::endl;
		}
		clients.push_back(clientSocket);
    }

    //_________________________JOIN_____________________________________________________________________________________________________________________________________________//
	for (int i = 0; i < nbTest; i++)
	{
		snprintf(CLIENT_REQ, sizeof(CLIENT_REQ), "JOIN #testSession\r\n");
		send(clients[i], CLIENT_REQ, strlen(CLIENT_REQ), 0);

		int bytes_received = recv(clients[i], SERVER_RES, sizeof(SERVER_RES), 0);
		if (bytes_received <= 0)
			std::cout << "Server connection closed or error occurred" << std::endl;
		std::string RES(SERVER_RES, bytes_received);
		if (RES.substr(0, 14) != ":localhost 332")
		{
			ko_count++;
			std::cout << RED << "JOIN TEST KO" << RESET << " for " << CLIENT_REQ << std::endl;
		}
	}

    //_________________________PRIVMSG_____________________________________________________________________________________________________________________________________________//
	for (int i = 0; i < nbTest; i++)
	{
		snprintf(CLIENT_REQ, sizeof(CLIENT_REQ), "PRIVMSG #testSession hi\r\n");
		send(clients[i], CLIENT_REQ, strlen(CLIENT_REQ), 0);

		int bytes_received = recv(clients[i], SERVER_RES, sizeof(SERVER_RES), 0);
		if (bytes_received <= 0)
			std::cout << "Server connection closed or error occurred." << std::endl;
	}
	//________________________MANUAL : replace "zac" and "zac_" with your clients nick on irssi___________________________________________________________
	
	//INVITE TEST
	snprintf(CLIENT_REQ, sizeof(CLIENT_REQ), "MODE #testSession +it\r\nINVITE zac #testSession\r\nINVITE zac_ #testSession\r\n");
	send(ownerAccountSession, CLIENT_REQ, strlen(CLIENT_REQ), 0);

	sleep(15);
	//KICK TEST
	snprintf(CLIENT_REQ, sizeof(CLIENT_REQ), "KICK #testSession zac u stink\r\n");
	send(ownerAccountSession, CLIENT_REQ, strlen(CLIENT_REQ), 0);

	//TOPIC TEST : FAIL
	snprintf(CLIENT_REQ, sizeof(CLIENT_REQ), "MODE #testSession +t\r\nTOPIC #testSession :zac u stink\r\n");
	send(ownerAccountSession, CLIENT_REQ, strlen(CLIENT_REQ), 0);

	//OPERATOR TEST : FAIL
	snprintf(CLIENT_REQ, sizeof(CLIENT_REQ), "MODE #testSession +o zac_\r\n");
	send(ownerAccountSession, CLIENT_REQ, strlen(CLIENT_REQ), 0);

	//OPERATOR SET MODE
	snprintf(CLIENT_REQ, sizeof(CLIENT_REQ), "MODE #testSession +k 42\r\n");
	send(ownerAccountSession, CLIENT_REQ, strlen(CLIENT_REQ), 0);

    //_________________________QUIT_____________________________________________________________________________________________________________________________________________//
	for (int i = 0; i < nbTest; i++)
	{
		snprintf(CLIENT_REQ, sizeof(CLIENT_REQ), "QUIT :my job here is done\r\n");
		send(clients[i], CLIENT_REQ, strlen(CLIENT_REQ), 0);

		int bytes_received = recv(clients[i], SERVER_RES, sizeof(SERVER_RES), 0);
		if (bytes_received <= 0)
			std::cout << "Server connection closed or error occurred." << std::endl;
	}

    if (ko_count == 0)
        std::cout << GREEN << "ALL TESTS OK" << RESET << std::endl;
    //_________________________END OF TESTING GROUND_____________________________________________________________________________________________________________________________________________//
   
    std::cout << "Enter any key to end the program." << YELLOW << "\nNote : Due to anti-flooding and the asynchronous nature of IRC servers, exiting before all messages are displayed by your client may end the test prematurily." << RESET << std::endl;
    std::string input;
    std::getline(std::cin, input);
	//FREE CLIENTS
	for (size_t i = 0; i < clients.size(); i++)
        close(clients[i]);
	close(ownerAccountSession);
    return 0;
}
