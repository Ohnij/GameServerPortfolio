#pragma once
#include <iostream>


inline int WSAError(const char* ErrorDiscription, int errorCode)
{
	//printf("%s (%d)", ErrorDiscription, errorCode);
	std::cerr << ErrorDiscription << errorCode << "\n";
	return errorCode;
}