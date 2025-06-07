#pragma once
#include "IOCPDefine.h"
#include <thread>


//IOCP ���Ƴֱ�
//https://learn.microsoft.com/ko-kr/cpp/standard-library/enable-shared-from-this-class?view=msvc-170
//shared_ptr ������ҋ� this�̷������� �ѱ�� 

//IocpServer
// 
// wsa����
// iocp��ü ����
// listener����

class Listener;
class IocpServer : public std::enable_shared_from_this<IocpServer>
{
public:
	IocpServer();
	~IocpServer();


	bool ServerStart(std::shared_ptr<class ClientAllocator> allocater = nullptr, int accept_count = 1);
	bool RegisterIOCP(HANDLE handle);
	bool Run();

	void OnAccept(OVERLAPPED_ACCEPT* accept_data, DWORD transferred_bytes);
	void OnRecive(OVERLAPPED_RECV* recv_data, DWORD transferred_bytes);
	void OnSend(OVERLAPPED_SEND* send_data, DWORD transferred_bytes);


	HANDLE _iocp_handle;
	std::shared_ptr<Listener> _listener;
	std::vector<std::thread> _worker_threads;
};
