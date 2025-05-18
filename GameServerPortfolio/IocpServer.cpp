#include "IocpServer.h"
#include "Listener.h"
#include "Client.h"
#include "Util.h"
#include "ClientManager.h"
#include <google/protobuf/message.h>


IocpServer::~IocpServer()
{
	//라이브러리 반환
	WSACleanup();
}

bool IocpServer::ServerStart()
{
	//Wsa 라이브러리 로딩

	WSADATA wsaData;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		WSAError("WSAStartUp 오류!! >> ", ::WSAGetLastError());
		return false;
	}

	_listener = std::make_shared<Listener>();
	if (_listener->Init(shared_from_this()) == false)
		return false;

	_iocp_handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	//IOCP객체 생성


	//IOCP에 Listen소켓 비동기로 받게 등록
	//CreateIoCompletionPort((HANDLE)_listenSocket, _iocpHandle, 0, 0);
	RegisterIOCP((HANDLE)_listener->GetSocket());

	//이제부터 받을꺼야~
	_listener->RegisterAccept();
	return true;
}

bool IocpServer::RegisterIOCP(HANDLE handle)
{
	auto result = CreateIoCompletionPort(handle, _iocp_handle, reinterpret_cast<ULONG_PTR>(handle), 0);
	if (result == NULL)
	{
		std::cerr << "[IOCP] 소켓 등록 실패! 에러 코드: " << GetLastError() << std::endl;
		return false;
	}
	return true;
}

bool IocpServer::Run()
{
	/*
	BOOL GetQueuedCompletionStatus(
	  [in]  HANDLE       CompletionPort,				//완료 포트에 대한 핸들입니다.
			LPDWORD      lpNumberOfBytesTransferred,	//완료된 I/O 작업에서 전송된 바이트 수를 받는 변수에 대한 포인터입니다.
	  [out] PULONG_PTR   lpCompletionKey,				//I/O 작업이 완료된 파일 핸들과 연결된 완료 키 값을 받는 변수에 대한 포인터입니다.
	  [out] LPOVERLAPPED *lpOverlapped,
	  [in]  DWORD        dwMilliseconds					//완료 패킷이 완료 포트에 나타날 때까지 호출자가 대기하려는 시간(밀리초)입니다.
	);
	*/
	DWORD transferred_byte = 0;
	ULONG_PTR key = 0;
	OVERLAPPED_EX* overlapped_data = nullptr;

	//INFINITE > 완료 통지가 올때까지 커널영역 접근 x
	BOOL res = GetQueuedCompletionStatus(_iocp_handle, &transferred_byte, &key, reinterpret_cast<LPOVERLAPPED*>(&overlapped_data), INFINITE);

	//비정상적종료:리턴값 false, 수신바이트 0
	//정상적인종료:리턴값 true, 수신바이트 0

	if (res)
	{
		switch (overlapped_data->type)
		{
			case IOCP_WORK::IOCP_ACCEPT: OnAccept(static_cast<OVERLAPPED_ACCEPT*>(overlapped_data), transferred_byte); break;
			case IOCP_WORK::IOCP_RECV: OnRecive(static_cast<OVERLAPPED_RECV*>(overlapped_data), transferred_byte); break;
			case IOCP_WORK::IOCP_SEND: OnSend(static_cast<OVERLAPPED_SEND*>(overlapped_data), transferred_byte); break;
			default:
			{

			}
		}
	}
	else
	{
		DWORD error = WSAGetLastError();
		switch (error)
		{
			case ERROR_NETNAME_DELETED:
			{
				//클라이언트 hard close 
				//하나하나 파싱해서 종료할것인가???
				if (key == 0 || key == INVALID_SOCKET)
				{
					
				}
				else
				{
					SOCKET socket = static_cast<SOCKET>(key);
					ClientManager::Instance().DisconnectClient(socket);
				}

			
				
			}
			break;
			case WAIT_TIMEOUT:
			{

			}
			break;
			default:
			{
				//서버 종료해야하는 오류?
			}
		}
		

	
	}

	return true;
}

void IocpServer::OnAccept(OVERLAPPED_ACCEPT* accept_data, DWORD transferred_bytes)
{
	_listener->ProcessAccept(accept_data);
}

void IocpServer::OnRecive(OVERLAPPED_RECV* recv_data, DWORD transferred_bytes)
{
	int client_id = recv_data->client_number;
	//TODO ::client 연결 종료 처리 해야함..
	if (transferred_bytes == 0)
	{
		//연결종료됨.
		ClientManager::Instance().DisconnectClient(recv_data->socket);
		return;
	}

	std::shared_ptr<Client> p_client = ClientManager::Instance().GetClientById(client_id);
	if (p_client == nullptr)
	{
		//TODO : 에러처리
		std::cerr << "클라이언트 " << client_id << " Nullptr !!!" << std::endl;
		return;
	}

	p_client->ProcessRecv(transferred_bytes);
}

void IocpServer::OnSend(OVERLAPPED_SEND* send_data, DWORD transferred_bytes)
{
	//OVERLAPPED_SEND* sendData = reinterpret_cast<OVERLAPPED_SEND*>(overlappedData);
	delete send_data;
}
