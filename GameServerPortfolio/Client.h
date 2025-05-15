#pragma once
#include "IOCPDefine.h"
#include <memory>


//Client
//
// 연결클라이언트 소켓관리 
// recive 버퍼 관리
// recive overlapped확장 관리
//

class Client : public std::enable_shared_from_this<Client>
{
public:
	Client();


	void SetClient(int client_number);
	void RegisterRecv();
	void RegisterSend(CHAR* p_data, int data_size);

	SOCKET _socket = INVALID_SOCKET;
	OVERLAPPED_RECV _recive;
	BYTE _recive_buffer[1024];
	int _client_number;
};