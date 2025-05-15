#pragma comment (lib, "ws2_32.lib")
#include "IocpServer.h"
#include <iostream>
#include <thread>

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
	std::shared_ptr<IocpServer> _iocpServer = std::make_shared<IocpServer>();
	if (_iocpServer->ServerStart() == false)
		return -1;

	
	//�����嵵 Ǯ�� ���� �����ؾ��ҵ�?
	std::thread workerThread([_iocpServer] {
		while (true)
		{
			_iocpServer->Run();
		}
	});

	if(workerThread.joinable())
		workerThread.join();

	std::cerr << "������ �����մϴ�" << std::endl;
	return 0;
}


