#pragma once
#include "Plat.h"

class Client
{
public:
	int ClientCode(void);
	int readMessage(char* buffer, int32_t size, SOCKET& asock);
	int Sendit(char* data, int32_t length, SOCKET asock);
};

