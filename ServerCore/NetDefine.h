#pragma once

#include <WinSock2.h>

constexpr int RECV_BUFFER_SIZE = 8192; // 8KB
constexpr int SEND_BUFFER_SIZE = 16384; // 16KB
constexpr int MAX_PACKET_SIZE = 8192;
constexpr int ACCEPT_BUFFER_SIZE = ((sizeof(SOCKADDR_IN) + 16) * 2);


//구조체 enum 기본타입 정의 전용
enum class IOCP_WORK
{
	IOCP_ACCEPT,
	IOCP_RECV,
	IOCP_SEND,
	IOCP_DISCONNECT,
};

//확장버전 나누기 (Type)
struct OVERLAPPED_EX : public OVERLAPPED
{
	void Reset()
	{
		ZeroMemory(this, sizeof(OVERLAPPED));
	}
	IOCP_WORK type;
};
struct OVERLAPPED_ACCEPT : public OVERLAPPED_EX
{
	SOCKET socket;
	BYTE accept_buffer[ACCEPT_BUFFER_SIZE];
};
struct OVERLAPPED_RECV : public OVERLAPPED_EX
{
	int iSessionID;
};
struct OVERLAPPED_SEND : OVERLAPPED_EX
{
public:
	int iSessionID;
};

struct OVERLAPPED_DISCONNECT : public OVERLAPPED_EX
{
};




typedef uint16_t PACKET_ID, PACKET_LEN;

#pragma pack(push, 1)
struct PacketHeader
{
	PACKET_LEN length;
	PACKET_ID ID;
};
#pragma pack(pop)