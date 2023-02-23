#pragma once
#include "Plat.h"

class Client
{
private:
	bool Ureg = false;
	char readmsg[256];
	char sending[256];
public:
	int ClientCode(void);
	int readMessage(char* buffer, int32_t size, SOCKET& asock);
	int Sendit(char* data, int32_t length, SOCKET asock);
};

