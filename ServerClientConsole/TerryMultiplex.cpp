// ServerClientConsole.cpp 
// By Phillip Croff
// message format is 1-byte for the message length followed by the message
//  ex:   6hello\0

#define _CRT_SECURE_NO_WARNINGS                 // turns of deprecated warnings
#define _WINSOCK_DEPRECATED_NO_WARNINGS         // turns of deprecated warnings for winsock
#include <winsock2.h>
//#include <ws2tcpip.h>                         // only need if you use inet_pton
#pragma comment(lib,"Ws2_32.lib")

#include <iostream>
#include <vector>
#include "definitions.h"
int tcp_recv_whole(SOCKET s, char* buf, int len);
int tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length);
int ServerCode(void);
int ClientCode(void);
int readMessage(char* buffer, int32_t size, SOCKET& asock);
int Sendit(char* data, int32_t length, SOCKET asock);
//int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, timeval* timeout);


int port;
int validate();

char readmsg[256];

struct regisUser {
	SOCKET userSocket;
	char User[15];
};

std::vector<regisUser> Users;

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
		ServerCode();

	}

	//Client
	if (choice == 2)
	{
		ClientCode();
	}

	return WSACleanup();
}

int validate()
{
	int x = 0;
	while (std::cin.fail())
	{
		std::cout << "not valid\n";
		std::cin.clear();
		std::cin.ignore(INT_MAX, '\n');
		std::cin >> x;
	}
	return x;
}
// loop recv func
int tcp_recv_whole(SOCKET s, char* buf, int len)
{
	int total = 0;

	do
	{
		int ret = recv(s, buf + total, len - total, 0);
		if (ret < 1)
			return ret;
		else
			total += ret;

	} while (total < len);

	return total;
}

// loop send func
int tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length)
{
	int result;
	int bytesSent = 0;

	while (bytesSent < length)
	{
		result = send(skSocket, (const char*)data + bytesSent, length - bytesSent, 0);

		if (result <= 0)
			return result;

		bytesSent += result;
	}

	return bytesSent;
}


int ServerCode(void)
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

	int numSock = 5;
	do
	{
		printf("How many clients should be supported?\n Will default to 3 if you dont put in anything valid\n");
		std::cin >> numSock;
		if (std::cin.fail())
		{
			numSock = 3;
			std::cin.clear();
			std::cin.ignore(INT_MAX, '\n');
		}
	} while (numSock < 1);


	//Socket
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET)
	{
		//printf("DEBUG// Socket function incorrect\n");
		int error = WSAGetLastError();
		printf("setUp error");
		return SETUP_ERROR;
	}

	//Bind
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);

	int result = bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (result == SOCKET_ERROR)
	{
		//DEBUG: Bind Incorrect
		int error = WSAGetLastError();
		printf("bind error");
		return BIND_ERROR;
	}

	//Listen
	result = listen(listenSocket, 1);
	if (result == SOCKET_ERROR)
	{
		//DEBUG: Listen Incorrect
		int error = WSAGetLastError();
		printf("Listen Error");
		return SETUP_ERROR;
	}

	printf("Waiting...\n\n");

	fd_set masterSet, readySet;
	FD_ZERO(&masterSet);
	FD_SET(listenSocket, &masterSet);
	FD_ZERO(&readySet);

	const timeval fixedtime = { 1,0 };


	SOCKET ComSocket;
	int close = 0;
	do
	{
		readySet = masterSet;
		int readyFD = select(0, &readySet, NULL, NULL, NULL);
		if (FD_ISSET(listenSocket, &readySet)) {
			ComSocket = accept(listenSocket, NULL, NULL);
			if (ComSocket == INVALID_SOCKET)
			{
				//DEBUG: Accept Incorrect
				int error = WSAGetLastError();
				return CONNECT_ERROR;
			}
			else if (ComSocket == SOCKET_ERROR)
			{
				int error = WSAGetLastError();
				return SHUTDOWN;
			}
			else if(masterSet.fd_count <= numSock)
			{
				FD_SET(ComSocket, &masterSet);
			}
			else
			{
				Sendit((char*)"This server is full", 40, ComSocket);
				printf("//Log in was attempted when master set was full");
				shutdown(ComSocket, SD_BOTH);
				closesocket(ComSocket);
			}
			/*if (ComSocket == INVALID_SOCKET)
			{
				printf("Accepted Socket\n");
				return;

			}
			else
			{
				
			}*/
		}

		for (int i = 0; i < masterSet.fd_count; i++)
		{
			if (FD_ISSET(masterSet.fd_array[i], &readySet))
			{
				if (masterSet.fd_array[i] != listenSocket) {
					memset(readmsg, 0, 256);
					int readout = readMessage(readmsg, 256, masterSet.fd_array[i]);

					if (readout == SHUTDOWN)
					{
						shutdown(masterSet.fd_array[i], SD_BOTH);
						closesocket(masterSet.fd_array[i]);
						FD_CLR(masterSet.fd_array[i], &masterSet);
					}
				}
			}
		}

	} while (close != 1);

	return SUCCESS;

}


int readMessage(char* buffer, int32_t size, SOCKET& asock) {
	//Communication

	uint8_t buffSize = size;
	int result = tcp_recv_whole(asock, (char*)&buffSize, 1);
	if ((result == SOCKET_ERROR))
	{
		int error = WSAGetLastError();
		//DEBUG: Read Is Incorrect
		return MESSAGE_ERROR;
	}
	else if (result == 0)
	{
		return SHUTDOWN;
	}

	if (size < strlen(buffer) || size <= 0)
	{
		//DEBUG: Message Size Error
		int error = WSAGetLastError();
		return PARAMETER_ERROR;
	}

	result = tcp_recv_whole(asock, buffer, buffSize);
	if ((result == SOCKET_ERROR))
	{
		int error = WSAGetLastError();
		//DEBUG: Read Is Incorrect
		return DISCONNECT;
	}
	else if (result == INVALID_SOCKET)
	{
		return SHUTDOWN;
	}
	printf("\n\n");
	for (size_t i = 0; i < Users.size(); i++)
	{
		if (Users[i].userSocket == asock)
		{
			printf("<User> ");
			printf(Users[i].User);
			printf(" : ");
		}
	}
	printf(buffer);
	printf("\n\n");
#pragma region junk

	//uint8_t bufferSize = size;
	//int size_result = tcp_recv_whole(asock, (char*)&bufferSize, 1); // receiveTcpData(server_Comm_Socket, (char*)&bufferSize, 1);
	//if ((size_result == SOCKET_ERROR))
	//{
	//	int error = WSAGetLastError();
	//	//DEBUG: Read Is Incorrect
	//	return;
	//}
	//else if (size_result == 0)
	//{
	//	return;
	//}


	//if (size < strlen(buffer) || size <= 0)
	//{
	//	//DEBUG: Message Size Error
	//	int error = WSAGetLastError();
	//	return;
	//}
	////DEBUG: Message Size Valid

	//int result = tcp_recv_whole(asock, buffer, bufferSize); // receiveTcpData(server_Comm_Socket, buffer, bufferSize);
	//if ((result == SOCKET_ERROR))
	//{
	//	int error = WSAGetLastError();
	//	//DEBUG: Read Is Incorrect
	//	return;
	//}
	//else if (result == INVALID_SOCKET)
	//{
	//	return;
	//}



	//printf("\n\n");
	//printf(buffer);
	//printf("\n\n");

#pragma endregion

	std::string sendmsg;
	sendmsg = buffer;
	if (sendmsg.find("$register") != std::string::npos && sendmsg.size() > 10)
	{
		int pos = sendmsg.find("$register");
		regisUser NewUser;
		strcpy(NewUser.User, sendmsg.substr(pos + 10, 15).c_str());
		NewUser.userSocket = asock;
		if (Users.empty())
		{
			Users.push_back(NewUser);
		}
		else
		{
			for (int i = 0; i < Users.size(); i++)
			{
				if (Users[i].User != NewUser.User)
				{
					if (Users[i].userSocket != NewUser.userSocket)
					{
						Users.push_back(NewUser);
						Sendit((char*)"New User has been registered", 50, asock);
					}
					else
					{
						strcpy(Users[i].User, NewUser.User);
						Sendit((char*)"Users name has been changed", 50, asock);
					}
				}
				else
				{
					if (Users[i].userSocket != NewUser.userSocket)
					{
						printf("This User already Exists \n");
					}
				}
			}
		}

	}

	if (strncmp(buffer, "$getlist", 11) == 0)
	{

		
		if (Users.empty() ==true)
		{
			Sendit((char*)"No registered Users", 40, asock);
			printf("the list is empty");
		}
		else
		{
			char text[256];
			memset(text, 0, 256);
			strcat(text, "Registered Users : ");
			printf("Registered Users : ");
			for (size_t i = 0; i < Users.size(); )
			{
				printf(Users[i].User);
				strcat(text, Users[i].User);
				i++;
				if (i < Users.size())
				{
					printf(", ");
					strcat(text, ", ");
				}
			}
			Sendit(text, strlen(text), asock);
		}
	}
	if (strncmp(buffer, "$quit", 11) == 0)
	{
		std::vector<regisUser>::iterator ptr;
		for (ptr = Users.begin(); ptr != Users.end(); ptr++)
		{
			if (ptr->userSocket == asock)
			{
				printf("<User> ");
				printf(ptr->User);
				printf(" disconnected");
				Users.erase(ptr);
				break;
			}
		}
		return SHUTDOWN;
	}
	return SUCCESS;
}

int Sendit(char* data, int32_t length, SOCKET asock) {
	uint8_t message_size = length;
	int size_result = tcp_send_whole(asock, (char*)&message_size, 1);
	if (size_result == SOCKET_ERROR)
	{
		//Debug: Send is incorrect
		int error = WSAGetLastError();
		return MESSAGE_ERROR;
	}
	else if (size_result == INVALID_SOCKET)
	{
		int error = WSAGetLastError();
		return SHUTDOWN;
	}

	if (length <= 0 || length > 256)
	{
		//DEBUG: Message Size Error
		int error = WSAGetLastError();
		return PARAMETER_ERROR;
	}
	//DEBUG: Message Size Valid

	int message_result = tcp_send_whole(asock, data, length);
	if (message_result == SOCKET_ERROR)
	{
		//Debug: Send is incorrect
		int error = WSAGetLastError();
		return DISCONNECT;
	}
	else if (message_result == INVALID_SOCKET)
	{
		int error = WSAGetLastError();
		return SHUTDOWN;
	}

	// Debug: Message Sent
	return SUCCESS;
}
int ClientCode(void)
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
	serverAddr.sin_port = htons(31337);

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
		

		std::cin.get(s, 256);
		std::cin.clear();
		std::cin.ignore(INT_MAX, '\n');

		Sendit(s, strlen(s), ComSocket);
		if (s == "$quit")
		{
			break;
		}

	} while (s != "$disconnect");
	shutdown(ComSocket, SD_BOTH);
	closesocket(ComSocket);
}
