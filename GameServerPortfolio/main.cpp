#pragma comment (lib, "ws2_32.lib")
#include "IocpServer.h"
#include <iostream>
#include <thread>
#include "PacketManager.h"
#include "DBConnection.h"

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
	//��Ŷ�ڵ鷯 �ʱ�ȭ (�̸����)
	PacketManager::Instance().InitPacketHandler();

	std::shared_ptr<IocpServer> _iocpServer = std::make_shared<IocpServer>();
	if (_iocpServer->ServerStart() == false)
		return -1;
	
	
	////�����嵵 Ǯ�� ���� �����ؾ��ҵ�?
	//std::thread workerThread([_iocpServer] {
	//	while (true)
	//	{
	//		_iocpServer->Run();
	//	}
	//});
	DBConnection dbc;
	if (false == dbc.Init())
	{
		std::cerr << "DB Error" << std::endl;
		return 0;
	}
	dbc.TestQuery();
	dbc.TestQuery2();

	while (true)
	{
		//std::this_thread::sleep_for(std::chrono::seconds(60));
		::Sleep(60000);
	}



	std::cerr << "������ �����մϴ�" << std::endl;
	return 0;
}


