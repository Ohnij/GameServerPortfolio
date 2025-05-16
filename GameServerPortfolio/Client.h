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
	~Client();

	void ResetObject(); //shared_ptr reset과 혼동하지 않도록!!!
	void SetClient(int client_number, SOCKET client_socket);
	void RegisterRecv();
	void ProcessRecv(int data_size);
	void RegisterSend(CHAR* p_data, int data_size);
	inline int GetClientID() { return _client_number; }
	inline SOCKET GetSocket() { return _socket; }
private:
	SOCKET _socket = INVALID_SOCKET;
	OVERLAPPED_RECV _recive;
	BYTE _recive_buffer[1024];
	int _client_number;
};