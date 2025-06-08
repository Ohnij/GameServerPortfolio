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
	//오브젝트 풀반납시 Manager에서 실행할것
	if (_socket != INVALID_SOCKET)
	{
		CancelIoEx(reinterpret_cast<HANDLE>(_socket), nullptr);
		shutdown(_socket, SD_BOTH); //이제 해당 소켓 송수신 안함!!
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
	//테스트 해보니까 flag를 상수 0으로 넣으면 nullptr참조로 에러남..
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
		//여기를 넘으면 패킷을 파싱을 할수 있다는거고, 읽어야한다는것. (실패시에도 해당 부분만큼은 읽어야함.)


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
	//경합?발생할수있다 병목 가능성 있음 is sending을 아토믹으로 하면서 여기선 읽어서 false체크만하고 return시키는방법도 좋아보임
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

	//커널 복사용 정보 (내부적으로 커널이 복사해가기때문에 지역변수라도 상관없다)
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
			return; //?오류상황?
		}
	}
	//챗지피티가 0이면 즉시완료되서 GQCS통지 안온다고하길래
	//ret =0 시 CompleteSend -> 타면 Queue많으면 무한재귀도 가능할것같아서
	//직접해제로 while(!queue empty)체크로 했는데 왠걸? 바로터짐 
	//즉시완료시에도 GQCS통지 오길래 항상 GQCS를 통해 send 반납하는것으로 수정함
}

void Client::CompleteSend(OVERLAPPED_SEND* overlepped_send)
{
	//완료시에만 (Queue비워주기~)
	{
		std::lock_guard<std::mutex> sendlock(_send_mutex);
		_send_queue.pop();
		_is_sending = false;
	}
	
	auto shard_ptr = overlepped_send->self;
	ReturnOverlapped(std::move(shard_ptr));
	ProcessSend(); //다시한번더 (락은 안에서 확인하기때문에 다른 스레드가 뺏어갈수도있음)
}

//참조 1이라도 없애기위해 RR참조 move이용
void Client::ReturnOverlapped(std::shared_ptr<OVERLAPPED_SEND>&& return_shared_ptr)
{
	//오버랩드매번 하길래 함수로 뺏다
	return_shared_ptr->self = nullptr;
	if (return_shared_ptr->buffer != nullptr)
	{
		return_shared_ptr->buffer->ResetBuffer();
		PoolManager::GetPool<SendBuffer>().Return(return_shared_ptr->buffer);
		return_shared_ptr->buffer = nullptr;
	}
	PoolManager::GetPool<OVERLAPPED_SEND>().Return(return_shared_ptr);

}
