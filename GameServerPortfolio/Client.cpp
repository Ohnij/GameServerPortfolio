#include "Client.h"
#include <iostream>
#include "Packet.h"
#include "jhnet.pb.h"

Client::Client()
	: _client_number(0)
{
	ZeroMemory(_recive_buffer, 1024);
	ZeroMemory(&_recive, sizeof(OVERLAPPED_RECV));
}

Client::~Client()
{
	ResetObject();
}

void Client::ResetObject()
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
	//char recive_data[1024];
	//ZeroMemory(recive_data, 1024);
	//CopyMemory(recive_data, _recive_buffer, size_t(data_size));
	//
	//std::cout << "[Ŭ�� " << _client_number << "]���� ������ : " << recive_data << "(" << data_size << " bytes)" << std::endl;

	////����..ó��
	//RegisterSend(reinterpret_cast<CHAR*>(recive_data), data_size);
	//RegisterRecv();

	if (data_size < sizeof(PacketHeader))
		return;

	PacketHeader* header = reinterpret_cast<PacketHeader*>(_recive_buffer);
	switch (static_cast<PACKET_TYPE>(header->packet_id))
	{

		//TODO: hashtable ��ŶŰ / �Լ���  ����
		case PACKET_TYPE::CS_ECHO:
		{
			if (data_size < sizeof(CSP_ECHO))
				return;

			CSP_ECHO* packet = reinterpret_cast<CSP_ECHO*>(_recive_buffer);
			std::string message(packet->message, 128);
			std::cout << "[echo] n: " << packet->number << " / m: " << message << std::endl;

			//����
			SCP_ECHO response = {};
			response.number = packet->number;
			strncpy_s(response.message, message.c_str(), sizeof(response.message));

			RegisterSend(reinterpret_cast<CHAR*>(&response), sizeof(response));
			break;
		}

		default:
			std::cout << "�˼����� ��Ŷ " << header->packet_id << std::endl;
			break;
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
