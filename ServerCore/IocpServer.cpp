#include "pch.h"
#include "IocpServer.h"
#include "NetUtil.h"
#include "Listener.h"
#include "SessionManager.h"
#include "Session.h"

IocpServer::IocpServer()
{
	
}

IocpServer::~IocpServer()
{
	for (auto& t : m_Workers)
	{
		if (t.joinable())
			t.join();
	}

	//���̺귯�� ��ȯ
	WSACleanup();
}


bool IocpServer::ServerStart(int iMaxClientSession, USHORT port)
{
	if (InitIocp() == false)
		return false;

	if (InitSocket(port) == false)
		return false;
	
	SessionManager::Instance().Init(iMaxClientSession);
	m_Listener->Register();

	//�������� ��������~
	RunWorkerThreads();

	
	return true;
}


bool IocpServer::SendMsg(int iSessionID, BYTE* pData, int iSize)
{
	
	Session* pSession = SESSION_MANAGER.Get(iSessionID);
	if (!pSession)
		return false;
	if (!pSession->IsConnected())
		return false;

	return pSession->RegisterSend(pData, iSize);
}




bool IocpServer::InitSocket(USHORT port)
{
	m_Listener = std::make_shared<Listener>();

	if (m_Listener->Init(port) == false)
		return false;

	if (RegisterIOCP((HANDLE)m_Listener->GetSocket(), 0) == false)
		return false;

	return true;
}

bool IocpServer::InitIocp()
{
	//Wsa ���̺귯�� �ε�
	WSADATA wsaData;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		WSAError("IOCP..Failed\nWSAStartUp ����!! >> ", ::WSAGetLastError());
		return false;
	}

	//IOCP��ü ����
	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	return true;
}

void IocpServer::RunWorkerThreads()
{

	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);
	//int threadCount = sys_info.dwNumberOfProcessors * 2;
	int iThreadCount = 4;

	for (int i = 0; i < iThreadCount; ++i)
	{
		m_Workers.emplace_back([this]()
		{
			this->WorkerThread();
			
		});
	}

}

bool IocpServer::RegisterIOCP(HANDLE handle, ULONG_PTR key)
{
	auto result = CreateIoCompletionPort(handle, m_hIocp, key, 0);
	if (result == NULL)
	{
		WSAError("IOCP ���� ��� ����! ���� �ڵ�: >> ", ::WSAGetLastError());
		//std::cerr << "[IOCP] ���� ��� ����! ���� �ڵ�: " << GetLastError() << std::endl;
		return false;
	}
	return true;
}



void IocpServer::WorkerThread()
{
	while (true)
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
		DWORD dwTransferredByte = 0;
		ULONG_PTR key = 0;
		Session* pSession = nullptr;
		OVERLAPPED_EX* pOverlappedEx = nullptr;

		//INFINITE > �Ϸ� ������ �ö����� Ŀ�ο��� ���� x
		BOOL res = GetQueuedCompletionStatus(m_hIocp,
			&dwTransferredByte,
			&key,
			reinterpret_cast<LPOVERLAPPED*>(&pOverlappedEx),
			INFINITE);

		pSession = reinterpret_cast<Session*>(key);

		//������������:���ϰ� false, ���Ź���Ʈ 0
		//������������:���ϰ� true, ���Ź���Ʈ 0
		if (res == false)
		{
			DWORD error = WSAGetLastError();
			switch (error)
			{

				case ERROR_NETNAME_DELETED:
				{
					//Ŭ���̾�Ʈ hard close 
					//�ϳ��ϳ� �Ľ��ؼ� �����Ұ��ΰ�???
					if (key != 0)
					{
						HandleDisconnect(pSession->GetSessionID());
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
			continue;
		}



		if (pOverlappedEx == nullptr)
		{
			WSAError("IOCP GQCS overlapped  nullptr!!!!!! >> ", ::WSAGetLastError());
			continue;
		}

		switch (pOverlappedEx->type)
		{
			case IOCP_WORK::IOCP_ACCEPT: HandleAccept(static_cast<OVERLAPPED_ACCEPT*>(pOverlappedEx), dwTransferredByte); break;
			case IOCP_WORK::IOCP_RECV: HandleRecv(static_cast<OVERLAPPED_RECV*>(pOverlappedEx), dwTransferredByte); break;
			case IOCP_WORK::IOCP_SEND: HandleSend(static_cast<OVERLAPPED_SEND*>(pOverlappedEx), dwTransferredByte); break;
			default:
			{

			}
		}
	}

}

void IocpServer::HandleAccept(OVERLAPPED_ACCEPT* pOverlappedAccept, DWORD dwTransferredBytes)
{
	Session* pSession = m_Listener->Process(pOverlappedAccept);
	if (pSession == nullptr)
		return;

	//Ŭ���̾�Ʈ ������ IOCP�� ��� (�񵿱� Recv / Send�ϱ�����) /->iocp���̰��ؾ���
	bool res = RegisterIOCP(
		reinterpret_cast<HANDLE>(pSession->GetSocket()), 
		reinterpret_cast<ULONG_PTR>(pSession));

	if (!res)
	{
		SESSION_MANAGER.CloseSession(pSession);
		return;
	}

	pSession->RegisterRecv();  // ���⼭ ���� Recv ����
}

void IocpServer::HandleRecv(OVERLAPPED_RECV* pOverlappedRecv, DWORD dwTransferredBytes)
{
	Session* pSession = SESSION_MANAGER.Get(pOverlappedRecv->m_iSessionID);
	if (!pSession || !pSession->IsConnected()) 
		return;

	if (dwTransferredBytes == 0)
	{
		HandleDisconnect(pOverlappedRecv->m_iSessionID);
		return;
	}
	pSession->ProcessRecv(dwTransferredBytes);

	const RecvBuffer& recv_buf = pSession->GetRecvBuffer();
	while (true)
	{
		BYTE* pReadPos = recv_buf.GetReadPos();
		int iDataSize = recv_buf.DataSize();

		if (iDataSize < sizeof(PacketHeader))
			break;

		PacketHeader* header = reinterpret_cast<PacketHeader*>(pReadPos);
		if (header->m_Length == 0 || MAX_PACKET_SIZE < header->m_Length)
		{
			//�߸�����Ŷ
			HandleDisconnect(pSession->GetSessionID());
			return;
		}

		//���� �� �ȿ����� �ƹ��۾� ����!!
		if (iDataSize < header->m_Length)
			break;

		// �ϼ��� ��Ŷ �� OnReceive
		OnReceive(pSession->GetSessionID(), pReadPos, header->m_Length);

		pSession->SuccessRecv(header->m_Length);
	}
	pSession->RegisterRecv(); // ���� ���� ���

}

void IocpServer::HandleSend(OVERLAPPED_SEND* pOverlappedSend, DWORD dwTransferredBytes)
{
	Session* pSession = SESSION_MANAGER.Get(pOverlappedSend->m_iSessionID);

	if (!pSession || !pSession->IsConnected())
		return;

	if (dwTransferredBytes == 0)
	{
		HandleDisconnect(pOverlappedSend->m_iSessionID);
		return;
	}
	pSession->ProcessSend(dwTransferredBytes);
	OnSend(pOverlappedSend->m_iSessionID, dwTransferredBytes);
}

void IocpServer::HandleDisconnect(int iSessionID)
{
	SESSION_MANAGER.CloseSession(SESSION_MANAGER.Get(iSessionID));
	OnDisconnect(iSessionID); // �ڽ� Ŭ���� �ݹ�
}

