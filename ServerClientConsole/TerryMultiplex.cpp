// ServerClientConsole.cpp 
// By Phillip Croff
// message format is 1-byte for the message length followed by the message
//  ex:   6hello\0
//


#include "Plat.h"
#include "Server.h"
#include "Client.h"


Server server;
Client client;



int main()
{
	WSADATA wsadata;
	WSAStartup(WINSOCK_VERSION, &wsadata);

	int choice;
	do
	{
		printf("Would you like to Create a Server or Client?\n");
		printf("1> Server\n");
		printf("2> Client\n");
		std::cin >> choice;
		validate();
		std::cin.clear();
		std::cin.ignore(INT_MAX, '\n');
	} while (choice != 1 && choice != 2);

	//Server
	if (choice == 1)
	{
		server.ServerCode();

	}

	//Client
	if (choice == 2)
	{
		client.ClientCode();
	}

	return WSACleanup();
}
