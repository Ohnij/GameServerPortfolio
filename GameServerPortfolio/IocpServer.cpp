#include "stdafx.h"
#include "IocpServer.h"
#include "Listener.h"
#include "Client.h"
#include "Util.h"
#include "ClientManager.h"
#include "ObjectPool.h"
#include "ClientAllocator.h"
#include <google/protobuf/message.h>


IocpServer::IocpServer()
{
	PoolManager::GetPool<Client>().ReserveObject(5000); //Ŭ���̾�Ʈ5000��
	PoolManager::GetPool<Client>().LimitCount(-1);		//5000�� ������ Ŭ���̾�Ʈ ���Ϲްڴ�.

}

IocpServer::~IocpServer()
{
	for (auto& t : _worker_threads)
	{
		if (t.joinable())
			t.join();
	}

	//���̺귯�� ��ȯ
	WSACleanup();
}

bool IocpServer::ServerStart(std::shared_ptr<class ClientAllocator> allocater, int accept_count )
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

	if (!allocater)
	{
		allocater = std::make_shared<ClientAllocator>();
	}	
	ClientManager::Instance().SetFactory(allocater);

	//IOCP��ü ����
	_iocp_handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);


	//IOCP�� Listen���� �񵿱�� �ް� ���
	//CreateIoCompletionPort((HANDLE)_listenSocket, _iocpHandle, 0, 0);
	RegisterIOCP((HANDLE)_listener->GetSocket());

	//�������� ��������~

	if (accept_count < 1) accept_count = 1;

	for (int i = 0; i < accept_count; ++i)
	{
		_listener->RegisterAccept();
	}

	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);
	//int threadCount = sys_info.dwNumberOfProcessors * 2;
	int threadCount = 4;
	for (int i = 0; i < threadCount; ++i)
	{
		_worker_threads.emplace_back([iocp = shared_from_this()]() { 
			//std::cerr << "WorkerThread On " << std::endl;
			while (true)
			{
				iocp->Run();
			}
		});
	}
	return true;
}

bool IocpServer::RegisterIOCP(HANDLE handle)
{
	auto result = CreateIoCompletionPort(handle, _iocp_handle, reinterpret_cast<ULONG_PTR>(handle), 0);
	if (result == NULL)
	{
		WSAError("IOCP ���� ��� ����! ���� �ڵ�: >> ", ::WSAGetLastError());
		//std::cerr << "[IOCP] ���� ��� ����! ���� �ڵ�: " << GetLastError() << std::endl;
		return false;
	}
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
	DWORD transferred_byte = 0;
	ULONG_PTR key = 0;
	OVERLAPPED_EX* overlapped_data = nullptr;

	//INFINITE > �Ϸ� ������ �ö����� Ŀ�ο��� ���� x
	BOOL res = GetQueuedCompletionStatus(_iocp_handle, &transferred_byte, &key, reinterpret_cast<LPOVERLAPPED*>(&overlapped_data), INFINITE);

	//������������:���ϰ� false, ���Ź���Ʈ 0
	//������������:���ϰ� true, ���Ź���Ʈ 0

	if (res)
	{
		if (overlapped_data == nullptr)
		{
			WSAError("IOCP GQCS overlapped  nullptr!!!!!! >> ", ::WSAGetLastError());
			return false;
		}

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
		if (overlapped_data && overlapped_data->type == IOCP_WORK::IOCP_SEND)
		{
			//�����ص� �޸� �� �ȳ���
			OnSend(static_cast<OVERLAPPED_SEND*>(overlapped_data), transferred_byte);
			return true;
		}

		switch (error)
		{
			case ERROR_NETNAME_DELETED:
			{
				//Ŭ���̾�Ʈ hard close 
				//�ϳ��ϳ� �Ľ��ؼ� �����Ұ��ΰ�???
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
				//���� �����ؾ��ϴ� ����?
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
	//TODO ::client ���� ���� ó�� �ؾ���..
	if (transferred_bytes == 0)
	{
		//���������.
		ClientManager::Instance().DisconnectClient(recv_data->socket);
		return;
	}

	std::shared_ptr<Client> p_client = ClientManager::Instance().GetClientById(client_id);
	if (p_client == nullptr)
	{
		//TODO : ����ó��
		std::cerr << "IOCP::OnRecive Ŭ���̾�Ʈ " << client_id << " Nullptr !!!" << std::endl;
		return;
	}

	p_client->ProcessRecv(transferred_bytes);
}

void IocpServer::OnSend(OVERLAPPED_SEND* send_data, DWORD transferred_bytes)
{
	//Queue��� ó�� shared_ptr Ǯ��ȯ �� ��ȯ���� ����
	std::shared_ptr<Client> p_client = ClientManager::Instance().GetClientBySocket(send_data->socket);
	if (p_client == nullptr)
	{
		//TODO : ����ó��
		std::cerr << "IOCP::OnSend Ŭ���̾�Ʈ Nullptr !!!" << std::endl;
		//�׷��� send�� ��Ե� ��ȯ�ؾ��Ѵ�.!!!
		auto shared = send_data->self;
		send_data->self = nullptr; //��ȯ���� ����
		if (send_data->buffer != nullptr)
		{
			send_data->buffer->ResetBuffer();
			PoolManager::GetPool<SendBuffer>().Return(send_data->buffer);
			send_data->buffer = nullptr; //��ȯ���� ����
		}
		PoolManager::GetPool<OVERLAPPED_SEND>().Return(shared);
		return;
	}
	else
	{
		//Ŭ���̾�Ʈ������ �˾Ƽ� ó������. �߰� queue������ ��۽ű���.
		p_client->CompleteSend(send_data);
	}


	//raw pointer �϶� ����
	////OVERLAPPED_SEND* sendData = reinterpret_cast<OVERLAPPED_SEND*>(overlappedData);
	//if (send_data->buffer != nullptr)
	//{
	//	send_data->buffer->ResetBuffer();
	//	PoolManager::GetPool<SendBuffer>().Return(send_data->buffer);
	//	send_data->buffer = nullptr;
	//}
	//delete send_data;
}
