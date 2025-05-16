#pragma once
#include <WinSock2.h>


#define PORT 7777

//구조체 enum 기본타입 정의 전용
#define ACCEPT_BUFFER_SIZE ((sizeof(SOCKADDR_IN) + 16) *2)


enum class IOCP_WORK
{
	IOCP_ACCEPT,
	IOCP_RECV,
	IOCP_SEND,
};

//확장버전 나누기 (Type)
struct OVERLAPPED_EX : public OVERLAPPED
{
	IOCP_WORK type;
};
struct OVERLAPPED_ACCEPT : public OVERLAPPED_EX
{
	SOCKET socket;
	BYTE accept_buffer[ACCEPT_BUFFER_SIZE];
};
struct OVERLAPPED_RECV : public OVERLAPPED_EX
{
	SOCKET socket;
	int client_number;
};

//일단은 이렇게 버퍼 물고있고, 나중에 버퍼를 관리해서 분배하는게 필요할듯
struct OVERLAPPED_SEND : OVERLAPPED_EX
{
public:
	BYTE send_buffer[1024];
};
