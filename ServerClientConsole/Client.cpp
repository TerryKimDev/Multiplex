#include "Client.h"


int Client::ClientCode(void)
{
	int port= 0;
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

int Client::readMessage(char* buffer, int32_t size, SOCKET& asock) {
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
	if (sendmsg.find("$register") != std::string::npos/* && sendmsg.size() > 10*/)
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


		if (Users.empty() == true)
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

	if (strncmp(buffer, "$getlog", 11) == 0)
	{


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

int Client::Sendit(char* data, int32_t length, SOCKET asock) {
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