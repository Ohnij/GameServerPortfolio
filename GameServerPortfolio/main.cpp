#pragma comment (lib, "ws2_32.lib")
#include "IocpServer.h"
#include <iostream>
#include <thread>
#include "PacketManager.h"

//#include <WinSock2.h>
//#include <MSWSock.h>
//
//#include <vector>
//#include <unordered_map>

/*
네이밍 규칙 
맴버변수는 _로시작
클래스/함수명 대문자로 띄어쓰기 구분 ShowMeTheMoeny
변수 _로 띄어쓰기 구분 (전부소문자) show_me_the_money
상수 _로띄어쓰기 구분 (전부대문자)
*/


int main()
{
	//패킷핸들러 초기화 (미리등록)
	PacketManager::Instance().InitPacketHandler();

	std::shared_ptr<IocpServer> _iocpServer = std::make_shared<IocpServer>();
	if (_iocpServer->ServerStart() == false)
		return -1;
	
	
	////스레드도 풀로 뭔가 관리해야할듯?
	//std::thread workerThread([_iocpServer] {
	//	while (true)
	//	{
	//		_iocpServer->Run();
	//	}
	//});

	while (true)
	{
		//std::this_thread::sleep_for(std::chrono::seconds(60));
		::Sleep(60000);
	}



	std::cerr << "서버를 종료합니다" << std::endl;
	return 0;
}


