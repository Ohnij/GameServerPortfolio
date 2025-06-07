#pragma once
#include "IOCPDefine.h"
#include <thread>


//IOCP 몰아넣기
//https://learn.microsoft.com/ko-kr/cpp/standard-library/enable-shared-from-this-class?view=msvc-170
//shared_ptr 를사용할떈 this이런식으로 넘기면 

//IocpServer
// 
// wsa세팅
// iocp객체 관리
// listener제어

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
