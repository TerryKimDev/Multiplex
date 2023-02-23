#pragma once

#define _CRT_SECURE_NO_WARNINGS                 // turns of deprecated warnings
#define _WINSOCK_DEPRECATED_NO_WARNINGS         // turns of deprecated warnings for winsock
#include <winsock2.h>
#pragma comment(lib,"Ws2_32.lib")
#include <iostream>
#include <vector>
#include <fstream>
#include "definitions.h"

//namespace Chat {
//	char readmsg[256];
//	char sending[256];
//}
struct regisUser {
	SOCKET userSocket;
	char User[15];
};
	static std::vector<regisUser> Users;

inline int validate()
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
};

inline int tcp_recv_whole(SOCKET s, char* buf, int len)
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
inline int tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length)
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


//unused
class Plat
{
public:
};

