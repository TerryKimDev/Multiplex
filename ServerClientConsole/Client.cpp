#include "Client.h"


int Client::ClientCode(void)
{
	do
	{
		printf("Please type out your port number.\nDefault will be 31337 should the input be invalid\n");
		std::cin >> port;
		if (std::cin.fail())
		{
			port = 31337;
			std::cin.clear();
			std::cin.ignore(INT_MAX, '\n');
		}


	} while (port <= 0 && port > 65353);

	std::string address;
	printf("Please give an Ip number to use\n type 1 to be given a default");
	std::cin >> address;
	if (address == "1")
	{
		address = "127.0.0.1";
	}
	//Socket
	SOCKET ComSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ComSocket == INVALID_SOCKET)
	{
		int error = WSAGetLastError();
		return SETUP_ERROR;
	}

	//Connect
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = inet_addr(address.c_str());
	serverAddr.sin_port = htons(port);

	int result = connect(ComSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (result == SOCKET_ERROR)
	{
		//connect error
		int error = WSAGetLastError();
		return CONNECT_ERROR;
	}
	else if (result == INVALID_SOCKET)
	{
		int error = WSAGetLastError();
		return SHUTDOWN;
	}
	else
	{
		printf("Connected\n");
	}

	char s[256];
	//Communication
	do
	{
		readMessage(readmsg, 256, ComSocket);


		std::cin.get(s, 256);
		std::cin.clear();
		std::cin.ignore(INT_MAX, '\n');

		Sendit(s, strlen(s), ComSocket);
		if (s == "$quit") //just extra security
		{
			break;
		}

	} while (s != "$quit");
	shutdown(ComSocket, SD_BOTH);
	closesocket(ComSocket);
}