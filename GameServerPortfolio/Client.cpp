#include "Client.h"
#include <iostream>
#include "Packet.h"
#include "jhnet.pb.h"
#include "PacketHandler.h"

Client::Client()
	: _client_number(0)
{
	ZeroMemory(_recive_buffer, 1024);
	ZeroMemory(&_recive, sizeof(OVERLAPPED_RECV));
}

Client::~Client()
{
	ResetClient();
}

void Client::ResetClient()
{
	//������Ʈ Ǯ�ݳ��� Manager���� �����Ұ�
	shutdown(_socket, SD_BOTH); //���� �ش� ���� �ۼ��� ����!!
	_client_number = 0;
	closesocket(_socket);
	_socket = INVALID_SOCKET;
}

void Client::SetClient(int client_number, SOCKET client_socket)
{
	ZeroMemory(_recive_buffer, 1024);
	ZeroMemory(&_recive, sizeof(OVERLAPPED_RECV));

	_client_number = client_number;
	_socket = client_socket;
	_recive.socket = client_socket;
}

void Client::RegisterRecv()
{
	ZeroMemory(_recive_buffer, 1024);
	ZeroMemory(&_recive, sizeof(OVERLAPPED_RECV));
	_recive.socket = _socket;
	_recive.type = IOCP_WORK::IOCP_RECV;
	_recive.client_number = _client_number;

	WSABUF wsabuf;
	wsabuf.buf = reinterpret_cast<CHAR*>(_recive_buffer);
	wsabuf.len = 1024;

	DWORD recvByte = 0;
	DWORD flag = 0;
	//�׽�Ʈ �غ��ϱ� flag�� ��� 0���� ������ nullptr������ ������..
	WSARecv(_socket, &wsabuf, 1, &recvByte, &flag, static_cast<OVERLAPPED*>(&_recive), nullptr);
}

void Client::ProcessRecv(int data_size)
{
	
	BYTE* data = _recive_buffer;
	while (data_size >= sizeof(PacketHeader))
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(data);

		if (data_size < header->packet_length)
			break;

		auto iter = g_PacketHandler.find(header->packet_id);
		if (iter != g_PacketHandler.end())
		{
			//client, byte*, int
			//TODO : ó�����??? �Ľ̸��������� ��� ������
			//�Լ������ͷ� �ѱ�⿣ ���� �̻��Ѵ��� Ȯ���� �б�� �����ϱ����ؼ� id, packet���� �����ʿ��ϰ�
			//���ݰ������ �Լ��ȿ��ٰ� ����ְ� �˾Ƽ� ó���ϴºκ��ε�,
			//��Ŷ�� ���� �Ľ��� Ȯ���ϰ� Read�ϷḦ �ؾ��ҵ�.
			iter->second(shared_from_this(),
				data + sizeof(PacketHeader),
				header->packet_length - sizeof(PacketHeader));
		}

		data += header->packet_length;
		data_size -= header->packet_length;
	}

	RegisterRecv();

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

void Client::SendPacket(BYTE* data, int size)
{
	RegisterSend((CHAR*)data, size);
}
