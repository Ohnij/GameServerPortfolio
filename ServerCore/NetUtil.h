#pragma once


inline int WSAError(const char* ErrorDiscription, int errorCode)
{
	//printf("%s (%d)", ErrorDiscription, errorCode);
	std::cerr << ErrorDiscription << errorCode << "\n";
	return errorCode;
}




inline int ErroLog()
{
#ifdef LOG_CONSOLE
	printf("")

#elseif LOG_FILE


#else 
#endif

}