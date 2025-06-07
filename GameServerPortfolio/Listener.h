#pragma once
#include "IOCPDefine.h"
#include <MSWSock.h>


//Listener
// 
// accept용 socket 관리
// accept된 클라이언트 iocp와연결 
// Client -> 클라이언트 소켓관리 
// accept용 Overlapped확장 구조체 관리

class IocpServer;
class Listener : public std::enable_shared_from_this<Listener>
{
public:
	Listener() = default;
	~Listener();
	

	bool Init(std::shared_ptr<IocpServer> iocp);
	SOCKET GetSocket() { return _listen_socket; }
	void InitAcceptSocket(SOCKET& socket);

	void RegisterAccept();
	void ProcessAccept(LPOVERLAPPED overlappedPtr);

	//연결 요청용 Listen소켓 세팅
	SOCKET _listen_socket;
	//OVERLAPPED_ACCEPT _accept_event;
	std::shared_ptr<IocpServer> _iocp;
};
