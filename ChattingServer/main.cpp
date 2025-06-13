#include "stdfx.h"

#include "ChattingServer.h"


int main()
{
	ChattingServer server;
	if(!server.Start(7777))
	{
		std::cerr << "Server Start Error\n";
		return -1;
	}
	std::cerr << "Server Start..\n";

	while (true)
	{
		::Sleep(60000);
	}
	return 0;
}