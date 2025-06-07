#include "stdafx.h"
#pragma comment (lib, "ws2_32.lib")
#include "IocpServer.h"
#include <thread>
#include "PacketManager.h"
#include "DBManager.h"
#include "GameClientAllocater.h"

//#include <WinSock2.h>
//#include <MSWSock.h>
//
//#include <vector>
//#include <unordered_map>

/*
���̹� ��Ģ 
�ɹ������� _�ν���
Ŭ����/�Լ��� �빮�ڷ� ���� ���� ShowMeTheMoeny
���� _�� ���� ���� (���μҹ���) show_me_the_money
��� _�ζ��� ���� (���δ빮��)
*/


int main()
{
	//��Ŷ�ڵ鷯 �ʱ�ȭ (�̸����) �����ڿ� �߰��� 
	//PacketManager::Instance().InitPacketHandler();

	std::shared_ptr<IocpServer> _iocpServer = std::make_shared<IocpServer>();
	auto allocator = std::make_shared<GameClientAllocater>();
	if (_iocpServer->ServerStart(allocator) == false)
		return -1;
	
	
	////�����嵵 Ǯ�� ���� �����ؾ��ҵ�?
	//std::thread workerThread([_iocpServer] {
	//	while (true)
	//	{
	//		_iocpServer->Run();
	//	}
	//});
	
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


