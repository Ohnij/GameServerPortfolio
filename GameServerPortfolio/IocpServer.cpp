#include "IocpServer.h"
#include "Listener.h"
#include "Client.h"
#include "Util.h"
#include "ClientManager.h"


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
	CreateIoCompletionPort(handle, _iocp_handle, 0, 0);
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
	DWORD transferredByte = 0;
	ULONG_PTR key = 0;
	OVERLAPPED_EX* overlappedData = nullptr;
	//여기서 클라이언트 객체로 뭔가가있어야할듯.
	BOOL res = GetQueuedCompletionStatus(_iocp_handle, &transferredByte, &key, reinterpret_cast<LPOVERLAPPED*>(&overlappedData), 1000);
	if (res)
	{
		switch (overlappedData->type)
		{
			case IOCP_WORK::IOCP_ACCEPT: OnAccept(static_cast<OVERLAPPED_ACCEPT*>(overlappedData), transferredByte); break;
			case IOCP_WORK::IOCP_RECV: OnRecive(static_cast<OVERLAPPED_RECV*>(overlappedData), transferredByte); break;
			case IOCP_WORK::IOCP_SEND: OnSend(static_cast<OVERLAPPED_SEND*>(overlappedData), transferredByte); break;
			default:
			{

			}
		}
	}
	else
	{

	}

	return true;
}

void IocpServer::OnAccept(OVERLAPPED_ACCEPT* accept_data, DWORD transferred_bytes)
{
	_listener->ProcessAccept(accept_data);
}

void IocpServer::OnRecive(OVERLAPPED_RECV* recv_data, DWORD transferred_bytes)
{
	int clientId = recv_data->client_number;
	//TODO ::client 연결 종료 처리 해야함..

	std::shared_ptr<Client> pClient = ClientManager::Instance().GetClientById(clientId);
	if (pClient == nullptr)
	{
		//TODO : 에러처리
		std::cerr << "클라이언트 " << clientId << " Nullptr !!!" << std::endl;
		return;
	}

	char recive_data[1024];
	ZeroMemory(recive_data, 1024);
	memcpy(recive_data, pClient->_recive_buffer, size_t(transferred_bytes));
	
	std::cout << "[클라 " << clientId << "]받은 데이터 : " << recive_data << "(" << transferred_bytes << " bytes)" << std::endl;

	//에코..처리
	pClient->RegisterSend(reinterpret_cast<CHAR*>(recive_data), transferred_bytes);
	pClient->RegisterRecv();
}

void IocpServer::OnSend(OVERLAPPED_SEND* send_data, DWORD transferred_bytes)
{
	//OVERLAPPED_SEND* sendData = reinterpret_cast<OVERLAPPED_SEND*>(overlappedData);
	delete send_data;
}
