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
	//오브젝트 풀반납시 Manager에서 실행할것
	shutdown(_socket, SD_BOTH); //이제 해당 소켓 송수신 안함!!
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

		//auto iter = g_PacketHandler.find(header->packet_id);
		auto func = PacketManager::Instance().GetPacketHandle(header->packet_id);
		if (func == nullptr)
		{
			//함수포인터 안나옴 (정의되지 않은 패킷 or 정의를 안했음 (사용자문제))
			//누적기록 해서 Ban / kick  (패킷 해킹시도 가능성)
		}
		else
		{
			func(shared_from_this(), _recive_buffer.GetReadPos() + sizeof(PacketHeader), header->packet_length - sizeof(PacketHeader));
		}

		//if (iter != g_PacketHandler.end())
		//{
		//	//client, byte*, int
		//	//TODO : 처리결과??? 파싱못했을경우는 어떻게 것인지
		//	//함수포인터로 넘기기엔 조금 이상한느낌 확실히 분기로 분할하기위해선 id, packet으로 수정필요하고
		//	//지금같은경우 함수안에다가 집어넣고 알아서 처리하는부분인데,
		//	//패킷을 먼저 파싱을 확인하고 Read완료를 해야할듯.
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
		//CopyMemory(sendOverlapped, p_data, data_size);	//데이터복사
		int write_size = (buffer->GetFreeSize() < data_size) ? buffer->GetFreeSize(): data_size;
		buffer->Write(p_data, write_size);


		//커널 복사용 정보 (내부적으로 커널이 복사해가기때문에 지역변수라도 상관없다)
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
