#pragma once
#include "IOCPDefine.h"
#include <MSWSock.h>


//Listener
// 
// accept�� socket ����
// accept�� Ŭ���̾�Ʈ iocp�Ϳ��� 
// Client -> Ŭ���̾�Ʈ ���ϰ��� 
// accept�� OverlappedȮ�� ����ü ����

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

	//���� ��û�� Listen���� ����
	SOCKET _listen_socket;
	//OVERLAPPED_ACCEPT _accept_event;
	std::shared_ptr<IocpServer> _iocp;
};
