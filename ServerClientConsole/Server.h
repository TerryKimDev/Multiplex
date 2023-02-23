#pragma once
#include "Plat.h"


class Server
{
private:
	char readmsg[256];
	char sending[256];
	FILE* thelog;
public:
	int ServerCode(void);
	int readMessage(char* buffer, int32_t size, SOCKET& asock);
	int Sendit(char* data, int32_t length, SOCKET asock);
};

