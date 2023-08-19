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
const int SERVER_PORT = 6666; // Replace with your actual server port

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
    if (ac < 4)
    {
        std::cout << "Usage : /<port> <password> <nb of users>\n" << std::endl;
        return 1;
    }

    // CHECK ARGUMENTS
    std::string SERVER_PASSWORD = av[2];
    int nbTest = atoi(av[3]);
    int ko_count = 0;
    std::vector<int> clients;
    if (nbTest <= 0 || SERVER_PASSWORD.empty())
    {
        std::cout << "Usage : /<port> <password> <nb of users>\n" << std::endl;
        return 1;
    }
    int SERVER_PORT = atoi(av[1]);

    // INIT TESTING SESSION
    int testingSession = initClient(av, SERVER_PORT, SERVER_PASSWORD);
    std::string nick = "TestingSessionClient";
    std::string uname = "session";
    std::string rname = "Testing Session";
    char CLIENT_REQ_SESSION[512] = {0};
    snprintf(CLIENT_REQ_SESSION, sizeof(CLIENT_REQ_SESSION), "PASS %s\r\nNICK %s\r\nUSER %s 0 * :%s\r\nJOIN #testSession\r\n", SERVER_PASSWORD.c_str(), nick.c_str(), uname.c_str(), rname.c_str());
    send(testingSession, CLIENT_REQ_SESSION, strlen(CLIENT_REQ_SESSION), 0);

    //_________________________TESTING GROUND_____________________________________________________________________________________________________________________________________________//
    //_________________________AUTHENTICATION_____________________________________________________________________________________________________________________________________________//
	for (int i = 0; i < nbTest ; i++)
    {
		// //Mitigate IRSSI flooding
		// //100ms
        // usleep(100000);
        {
			//INIT CLIENTS
            int clientSocket = initClient(av, SERVER_PORT, SERVER_PASSWORD);
            if (clientSocket == -1)
            {
                std::cout << "Error : couldn't init client" << std::endl;
                return 1;
            }
            std::string nickname = "TesterClient";
            std::string username = "test_client";
            std::string realname = "Tester Client";
            char CLIENT_REQ[2048] = {0};
            char SERVER_RES[512] = {0};

            //AUTH TEST
			{
				snprintf(CLIENT_REQ, sizeof(CLIENT_REQ), "PASS %s\r\nNICK %s\r\nUSER %s 0 * :%s\r\n", SERVER_PASSWORD.c_str(), nickname.c_str(), username.c_str(), realname.c_str());
				send(clientSocket, CLIENT_REQ, strlen(CLIENT_REQ), 0);

				int bytes_received = recv(clientSocket, SERVER_RES, sizeof(SERVER_RES), 0);
				if (bytes_received <= 0)
					std::cout << "Server connection closed or error occurred." << std::endl;
				std::string RES(SERVER_RES, bytes_received);
				if (RES.substr(0, 14) != ":localhost 001")
				{
					ko_count++;
					std::cout << RED << "AUTH TEST KO" << RESET << " for " << CLIENT_REQ << std::endl;
				}
			}

			//JOIN TEST
			{
				snprintf(CLIENT_REQ, sizeof(CLIENT_REQ), "JOIN #testSession\r\n");
           		send(clientSocket, CLIENT_REQ, strlen(CLIENT_REQ), 0);

				int bytes_received = recv(clientSocket, SERVER_RES, sizeof(SERVER_RES), 0);
				if (bytes_received <= 0)
					std::cout << "Server connection closed or error occurred." << std::endl;
				std::string RES(SERVER_RES, bytes_received);
				if (RES.substr(0, 14) != ":localhost 332")
				{
					ko_count++;
					std::cout << RED << "JOIN TEST KO" << RESET << " for " << CLIENT_REQ << std::endl;
				}
			}

			//PRIVMSG TEST
			{
				snprintf(CLIENT_REQ, sizeof(CLIENT_REQ), "PRIVMSG #testSession hi\r\n");
           		send(clientSocket, CLIENT_REQ, strlen(CLIENT_REQ), 0);

				int bytes_received = recv(clientSocket, SERVER_RES, sizeof(SERVER_RES), 0);
				if (bytes_received <= 0)
					std::cout << "Server connection closed or error occurred." << std::endl;
			}

            clients.push_back(clientSocket);
        }
    }

    if (ko_count == 0)
        std::cout << GREEN << "ALL TESTS OK" << RESET << std::endl;
    //_________________________TESTING GROUND_____________________________________________________________________________________________________________________________________________//
    // STANDBY TO AVOID TRIGGERING SIGPIPE
	sleep(5);
    // CLEAN CLIENT FDS
    for (size_t i = 0; i < clients.size(); i++)
        close(clients[i]);
	close(testingSession);
    return 0;
}
