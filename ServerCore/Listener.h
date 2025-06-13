#pragma once
#include "IOCPDefine.h"
#include <MSWSock.h>


//Listener
// 
// accept용 socket 관리
// accept된 클라이언트 iocp와연결 
// Client -> 클라이언트 소켓관리 
// accept용 Overlapped확장 구조체 관리

class Session;
class Listener
{
public:
	Listener() = default;
	~Listener() = default;


	bool Init(USHORT port);						//리스너 초기설정
	SOCKET GetSocket() { return m_Socket; }

	void PrepareAcceptSocket(SOCKET& socket);	//Accept걸기전 소캣설정하기 (세션이 쓸 소캣)


	void Register();							//최초 1회 ~이후 자동실행
	Session* Process(OVERLAPPED_ACCEPT* acceptData); //accept설정후 접속 세션반환하기
private:

	//연결 요청용 Listen소켓 세팅
	SOCKET m_Socket;
};
