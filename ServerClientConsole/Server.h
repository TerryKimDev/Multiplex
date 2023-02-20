#pragma once
#include "Plat.h"

FILE* thelog;

class Server
{

public:
	int ServerCode(void);
	int readMessage(char* buffer, int32_t size, SOCKET& asock);
	int Sendit(char* data, int32_t length, SOCKET asock);
};

