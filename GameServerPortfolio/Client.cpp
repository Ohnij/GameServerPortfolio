#include "stdafx.h"
#include "Client.h"
#include "Packet.h"
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
	if (_socket != INVALID_SOCKET)
	{
		CancelIoEx(reinterpret_cast<HANDLE>(_socket), nullptr);
		shutdown(_socket, SD_BOTH); //���� �ش� ���� �ۼ��� ����!!
		_client_number = 0;
		closesocket(_socket);
	}
	
	while (!_send_queue.empty())
	{
		auto ret = _send_queue.front();
		_send_queue.pop();
		ReturnOverlapped(std::move(ret));
	}
		
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


		OnReceive(_recive_buffer.GetReadPos(), header->packet_length);

		_recive_buffer.Read(header->packet_length);
		//data += header->packet_length;
		data_size -= header->packet_length;
	}

	RegisterRecv();

}


void Client::OnReceive(BYTE* data, int size)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(data);
	BYTE* payload = reinterpret_cast<BYTE*>(data + sizeof(PacketHeader));
	int payload_size = size - sizeof(PacketHeader);
	std::string receive_data(reinterpret_cast<char*>(payload), payload_size);
	
	std::cerr << "Recv [" << header->packet_id << "] : " << receive_data << "(" << size << ")\n";
}



void Client::RegisterSend(BYTE* p_data, int data_size)
{
	while (0 < data_size)
	{
		auto buffer = PoolManager::GetPool<SendBuffer>().Get();
		buffer->ResetBuffer();

		//OVERLAPPED_SEND* sendOverlapped = new OVERLAPPED_SEND();
		auto sendOverlapped = PoolManager::GetPool<OVERLAPPED_SEND>().Get();
		sendOverlapped->type = IOCP_WORK::IOCP_SEND;	//_ex
		sendOverlapped->buffer = buffer;
		sendOverlapped->self = sendOverlapped;
		sendOverlapped->socket = _socket;
		int write_size = (buffer->GetFreeSize() < data_size) ? buffer->GetFreeSize(): data_size;
		buffer->Write(p_data, write_size);

		p_data += write_size;
		data_size -= write_size;
		{
			std::lock_guard<std::mutex> sendlock(_send_mutex);
			_send_queue.push(sendOverlapped);
		}
	}

	ProcessSend();
}

void Client::ProcessSend()
{
	//����?�߻��Ҽ��ִ� ���� ���ɼ� ���� is sending�� ��������� �ϸ鼭 ���⼱ �о falseüũ���ϰ� return��Ű�¹���� ���ƺ���
	std::lock_guard<std::mutex> sendlock(_send_mutex);
	if (_is_sending)
	{
		return;
	}
	_is_sending = true;
	if (_send_queue.empty())
	{
		_is_sending = false;
		return;
	}

	auto peek = _send_queue.front();

	//Ŀ�� ����� ���� (���������� Ŀ���� �����ذ��⶧���� ���������� �������)
	WSABUF wsaSendBuf;
	//wsaSendBuf.buf = reinterpret_cast<CHAR*>(sendOverlapped->send_buffer);
	wsaSendBuf.buf = reinterpret_cast<CHAR*>(peek->buffer->GetBuffer());
	wsaSendBuf.len = peek->buffer->GetWriteSize();

	DWORD sendByte = 0;
	DWORD flag = 0;
	int ret = WSASend(_socket, &wsaSendBuf, 1, &sendByte, flag, static_cast<OVERLAPPED*>(peek.get()), nullptr);
	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != WSA_IO_PENDING)
		{
			std::cerr << "[Send Error] code: " << err << std::endl;
			_send_queue.pop();
			_is_sending = false;
			ReturnOverlapped(std::move(peek));
			return; //?������Ȳ?
		}
	}
	//ê����Ƽ�� 0�̸� ��ÿϷ�Ǽ� GQCS���� �ȿ´ٰ��ϱ淡
	//ret =0 �� CompleteSend -> Ÿ�� Queue������ ������͵� �����ҰͰ��Ƽ�
	//���������� while(!queue empty)üũ�� �ߴµ� �ذ�? �ٷ����� 
	//��ÿϷ�ÿ��� GQCS���� ���淡 �׻� GQCS�� ���� send �ݳ��ϴ°����� ������
}

void Client::CompleteSend(OVERLAPPED_SEND* overlepped_send)
{
	//�Ϸ�ÿ��� (Queue����ֱ�~)
	{
		std::lock_guard<std::mutex> sendlock(_send_mutex);
		_send_queue.pop();
		_is_sending = false;
	}
	
	auto shard_ptr = overlepped_send->self;
	ReturnOverlapped(std::move(shard_ptr));
	ProcessSend(); //�ٽ��ѹ��� (���� �ȿ��� Ȯ���ϱ⶧���� �ٸ� �����尡 �����������)
}

//���� 1�̶� ���ֱ����� RR���� move�̿�
void Client::ReturnOverlapped(std::shared_ptr<OVERLAPPED_SEND>&& return_shared_ptr)
{
	//��������Ź� �ϱ淡 �Լ��� ����
	return_shared_ptr->self = nullptr;
	if (return_shared_ptr->buffer != nullptr)
	{
		return_shared_ptr->buffer->ResetBuffer();
		PoolManager::GetPool<SendBuffer>().Return(return_shared_ptr->buffer);
		return_shared_ptr->buffer = nullptr;
	}
	PoolManager::GetPool<OVERLAPPED_SEND>().Return(return_shared_ptr);

}
