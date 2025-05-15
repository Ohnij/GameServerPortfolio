#include "IocpServer.h"
#include "Listener.h"
#include "Client.h"
#include "Util.h"
#include "ClientManager.h"


IocpServer::~IocpServer()
{
	//���̺귯�� ��ȯ
	WSACleanup();
}

bool IocpServer::ServerStart()
{
	//Wsa ���̺귯�� �ε�

	WSADATA wsaData;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		WSAError("WSAStartUp ����!! >> ", ::WSAGetLastError());
		return false;
	}

	_listener = std::make_shared<Listener>();
	if (_listener->Init(shared_from_this()) == false)
		return false;

	_iocp_handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	//IOCP��ü ����


	//IOCP�� Listen���� �񵿱�� �ް� ���
	//CreateIoCompletionPort((HANDLE)_listenSocket, _iocpHandle, 0, 0);
	RegisterIOCP((HANDLE)_listener->GetSocket());

	//�������� ��������~
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
	  [in]  HANDLE       CompletionPort,				//�Ϸ� ��Ʈ�� ���� �ڵ��Դϴ�.
			LPDWORD      lpNumberOfBytesTransferred,	//�Ϸ�� I/O �۾����� ���۵� ����Ʈ ���� �޴� ������ ���� �������Դϴ�.
	  [out] PULONG_PTR   lpCompletionKey,				//I/O �۾��� �Ϸ�� ���� �ڵ�� ����� �Ϸ� Ű ���� �޴� ������ ���� �������Դϴ�.
	  [out] LPOVERLAPPED *lpOverlapped,
	  [in]  DWORD        dwMilliseconds					//�Ϸ� ��Ŷ�� �Ϸ� ��Ʈ�� ��Ÿ�� ������ ȣ���ڰ� ����Ϸ��� �ð�(�и���)�Դϴ�.
	);
	*/
	DWORD transferredByte = 0;
	ULONG_PTR key = 0;
	OVERLAPPED_EX* overlappedData = nullptr;
	//���⼭ Ŭ���̾�Ʈ ��ü�� �������־���ҵ�.
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
	//TODO ::client ���� ���� ó�� �ؾ���..

	std::shared_ptr<Client> pClient = ClientManager::Instance().GetClientById(clientId);
	if (pClient == nullptr)
	{
		//TODO : ����ó��
		std::cerr << "Ŭ���̾�Ʈ " << clientId << " Nullptr !!!" << std::endl;
		return;
	}

	char recive_data[1024];
	ZeroMemory(recive_data, 1024);
	memcpy(recive_data, pClient->_recive_buffer, size_t(transferred_bytes));
	
	std::cout << "[Ŭ�� " << clientId << "]���� ������ : " << recive_data << "(" << transferred_bytes << " bytes)" << std::endl;

	//����..ó��
	pClient->RegisterSend(reinterpret_cast<CHAR*>(recive_data), transferred_bytes);
	pClient->RegisterRecv();
}

void IocpServer::OnSend(OVERLAPPED_SEND* send_data, DWORD transferred_bytes)
{
	//OVERLAPPED_SEND* sendData = reinterpret_cast<OVERLAPPED_SEND*>(overlappedData);
	delete send_data;
}
