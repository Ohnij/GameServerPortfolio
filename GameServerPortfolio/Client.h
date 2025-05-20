#pragma once
#include "IOCPDefine.h"
#include <memory>
#include "RecvBuffer.h"
#include "SendBuffer.h"

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

	void ResetClient(); //shared_ptr reset�� ȥ������ �ʵ���!!! (������ �������)-> ����Ŭ����
	void SetClient(int client_number, SOCKET client_socket);
	void RegisterRecv();
	void ProcessRecv(int data_size);
	void RegisterSend(BYTE* p_data, int data_size);

	void SendPacket(BYTE* data, int size);
	inline int GetClientID() { return _client_number; }
	inline SOCKET GetSocket() { return _socket; }
private:
	SOCKET _socket = INVALID_SOCKET;
	OVERLAPPED_RECV _recive;
	RecvBuffer _recive_buffer;
	//BYTE _recive_buffer[1024];

	

	int _client_number;
};