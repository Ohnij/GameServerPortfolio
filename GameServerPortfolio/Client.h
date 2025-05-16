#pragma once
#include "IOCPDefine.h"
#include <memory>


//Client
//
// ����Ŭ���̾�Ʈ ���ϰ��� 
// recive ���� ����
// recive overlappedȮ�� ����
//

class Client : public std::enable_shared_from_this<Client>
{
public:
	Client();
	~Client();

	void ResetObject(); //shared_ptr reset�� ȥ������ �ʵ���!!!
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