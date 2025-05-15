#include "Client.h"



Client::Client()
	: _client_number(0)
{
	ZeroMemory(_recive_buffer, 1024);
	ZeroMemory(&_recive, sizeof(OVERLAPPED_RECV));
}

void Client::SetClient(int client_number)
{
	_client_number = client_number;
	_recive.client_number = client_number;
}

void Client::RegisterRecv()
{
	ZeroMemory(_recive_buffer, 1024);
	ZeroMemory(&_recive, sizeof(OVERLAPPED_RECV));
	_recive.client_number = _client_number;
	_recive.type = IOCP_WORK::IOCP_RECV;

	WSABUF wsabuf;
	wsabuf.buf = reinterpret_cast<CHAR*>(_recive_buffer);
	wsabuf.len = 1024;

	DWORD recvByte = 0;
	DWORD flag = 0;
	//�׽�Ʈ �غ��ϱ� flag�� ��� 0���� ������ nullptr������ ������..
	WSARecv(_socket, &wsabuf, 1, &recvByte, &flag, static_cast<OVERLAPPED*>(&_recive), nullptr);
}

void Client::RegisterSend(CHAR* p_data, int data_size)
{
	OVERLAPPED_SEND* sendOverlapped = new OVERLAPPED_SEND();
	sendOverlapped->type = IOCP_WORK::IOCP_SEND;	//_ex
	ZeroMemory(sendOverlapped->send_buffer, 1024);	//_send
	CopyMemory(sendOverlapped->send_buffer, p_data, data_size);	//�����ͺ���

	//Ŀ�� ����� ���� (���������� Ŀ���� �����ذ��⶧���� ���������� �������)
	WSABUF wsaSendBuf;
	wsaSendBuf.buf = reinterpret_cast<CHAR*>(sendOverlapped->send_buffer);
	wsaSendBuf.len = data_size;

	DWORD sendByte = data_size;
	DWORD flag = 0;

	WSASend(_socket, &wsaSendBuf, 1, &sendByte, flag, static_cast<OVERLAPPED*>(sendOverlapped), nullptr);

}
