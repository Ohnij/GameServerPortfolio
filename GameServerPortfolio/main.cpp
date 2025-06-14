#include "stdafx.h"
#pragma comment (lib, "ws2_32.lib")
#include "GameServer.h"
#include <thread>
#include "DBManager.h"

/*
네이밍 규칙 
맴버변수는 m_로시작
클래스/함수명 대문자로 띄어쓰기 구분 ShowMeTheMoeny
변수 대문자로 띄어쓰기 구분 / 변수 타입에 맞는 

지역,stuct변수는 typeName으로
class변수는 m_typeName
i=int 
dw= DWORD
str=string
ws =wstring
sz =char zero ..(string)
wz =wchar zero
p = pointer
..
상수 _로띄어쓰기 구분 (전부대문자)


*/

int main()
{
	std::shared_ptr<GameServer> _iocpServer = std::make_shared<GameServer>();
	if (_iocpServer->Start(7777) == false)
		return -1;
	

	if (DBManager::Instance().Init() == false)
	{
		std::cerr << "DB Error\n";
		return 0;

	}
	DBManager::Instance().DBTest();

	while (true)
	{
		//std::this_thread::sleep_for(std::chrono::seconds(60));
		::Sleep(60000);
	}



	std::cerr << "서버를 종료합니다\n";
	return 0;
}


