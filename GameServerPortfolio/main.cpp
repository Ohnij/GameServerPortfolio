#include "stdafx.h"
#pragma comment (lib, "ws2_32.lib")
#include "GameServer.h"
#include <thread>
#include "DBManager.h"

/*
���̹� ��Ģ 
�ɹ������� m_�ν���
Ŭ����/�Լ��� �빮�ڷ� ���� ���� ShowMeTheMoeny
���� �빮�ڷ� ���� ���� / ���� Ÿ�Կ� �´� 

����,stuct������ typeName����
class������ m_typeName
i=int 
dw= DWORD
str=string
ws =wstring
sz =char zero ..(string)
wz =wchar zero
p = pointer
..
��� _�ζ��� ���� (���δ빮��)


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



	std::cerr << "������ �����մϴ�\n";
	return 0;
}


