#include "Client.h"
#include <iostream>
#include "Packet.h"
#include "jhnet.pb.h"
#include "PacketManager.h"
#include "ObjectPool.h"

Client::Client()
	: _client_number(0)
{
	//ZeroMemory(_recive_buffer, 1024);
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
	//ZeroMemory(_recive_buffer, 1024);
	ZeroMemory(&_recive, sizeof(OVERLAPPED_RECV));

	_client_number = client_number;
	_socket = client_socket;
	_recive.socket = client_socket;
}

void Client::RegisterRecv()
{
	//ZeroMemory(_recive_buffer, 1024);
	ZeroMemory(&_recive, sizeof(OVERLAPPED_RECV));
	_recive.socket = _socket;
	_recive.type = IOCP_WORK::IOCP_RECV;
	_recive.client_number = _client_number;

	WSABUF wsabuf;
	wsabuf.buf = reinterpret_cast<CHAR*>(_recive_buffer.GetWritePos());
	wsabuf.len = _recive_buffer.FreeSize();

	DWORD recvByte = 0;
	DWORD flag = 0;
	//�׽�Ʈ �غ��ϱ� flag�� ��� 0���� ������ nullptr������ ������..
	WSARecv(_socket, &wsabuf, 1, &recvByte, &flag, static_cast<OVERLAPPED*>(&_recive), nullptr);
}

void Client::ProcessRecv(int data_size)
{
	_recive_buffer.Write(data_size);
	while (data_size >= sizeof(PacketHeader))
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(_recive_buffer.GetReadPos());

		if (data_size < header->packet_length)
			break;
		//���⸦ ������ ��Ŷ�� �Ľ��� �Ҽ� �ִٴ°Ű�, �о���Ѵٴ°�. (���нÿ��� �ش� �κи�ŭ�� �о����.)

		//auto iter = g_PacketHandler.find(header->packet_id);
		auto func = PacketManager::Instance().GetPacketHandle(header->packet_id);
		if (func == nullptr)
		{
			//�Լ������� �ȳ��� (���ǵ��� ���� ��Ŷ or ���Ǹ� ������ (����ڹ���))
			//������� �ؼ� Ban / kick  (��Ŷ ��ŷ�õ� ���ɼ�)
		}
		else
		{
			func(shared_from_this(), _recive_buffer.GetReadPos() + sizeof(PacketHeader), header->packet_length - sizeof(PacketHeader));
		}

		//if (iter != g_PacketHandler.end())
		//{
		//	//client, byte*, int
		//	//TODO : ó�����??? �Ľ̸��������� ��� ������
		//	//�Լ������ͷ� �ѱ�⿣ ���� �̻��Ѵ��� Ȯ���� �б�� �����ϱ����ؼ� id, packet���� �����ʿ��ϰ�
		//	//���ݰ������ �Լ��ȿ��ٰ� ����ְ� �˾Ƽ� ó���ϴºκ��ε�,
		//	//��Ŷ�� ���� �Ľ��� Ȯ���ϰ� Read�ϷḦ �ؾ��ҵ�.
		//	iter->second(shared_from_this(),
		//		data + sizeof(PacketHeader),
		//		header->packet_length - sizeof(PacketHeader));
		//}
		_recive_buffer.Read(header->packet_length);
		//data += header->packet_length;
		data_size -= header->packet_length;
	}

	RegisterRecv();

}

void Client::RegisterSend(BYTE* p_data, int data_size)
{
	while (0 < data_size)
	{
		auto buffer = PoolManager::GetPool<SendBuffer>().Get();
		buffer->ResetBuffer();

		OVERLAPPED_SEND* sendOverlapped = new OVERLAPPED_SEND();
		sendOverlapped->type = IOCP_WORK::IOCP_SEND;	//_ex
		sendOverlapped->buffer = buffer;
		//ZeroMemory(sendOverlapped->send_buffer, 1024);	//_send
		//CopyMemory(sendOverlapped, p_data, data_size);	//�����ͺ���
		int write_size = (buffer->GetFreeSize() < data_size) ? buffer->GetFreeSize(): data_size;
		buffer->Write(p_data, write_size);


		//Ŀ�� ����� ���� (���������� Ŀ���� �����ذ��⶧���� ���������� �������)
		WSABUF wsaSendBuf;
		//wsaSendBuf.buf = reinterpret_cast<CHAR*>(sendOverlapped->send_buffer);
		wsaSendBuf.buf = reinterpret_cast<CHAR*>(buffer->GetBuffer());
		wsaSendBuf.len = write_size;

		DWORD sendByte = 0;
		DWORD flag = 0;
		p_data += write_size;
		data_size -= write_size;
		int ret = WSASend(_socket, &wsaSendBuf, 1, &sendByte, flag, static_cast<OVERLAPPED*>(sendOverlapped), nullptr);
		if (ret == SOCKET_ERROR)
		{
			int err = WSAGetLastError();
			if (err != WSA_IO_PENDING)
			{
				std::cerr << "[Send Error] code: " << err << std::endl;
				delete sendOverlapped;
				break;
			}
		}
	}
}

void Client::SendPacket(BYTE* data, int size)
{
	RegisterSend(data, size);
}
