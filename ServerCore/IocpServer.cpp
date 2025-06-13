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

	//라이브러리 반환
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

	//이제부터 받을꺼야~
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
	//Wsa 라이브러리 로딩
	WSADATA wsaData;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		WSAError("IOCP..Failed\nWSAStartUp 오류!! >> ", ::WSAGetLastError());
		return false;
	}

	//IOCP객체 생성
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
		WSAError("IOCP 소켓 등록 실패! 에러 코드: >> ", ::WSAGetLastError());
		//std::cerr << "[IOCP] 소켓 등록 실패! 에러 코드: " << GetLastError() << std::endl;
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
  [in]  HANDLE       CompletionPort,				//완료 포트에 대한 핸들입니다.
		LPDWORD      lpNumberOfBytesTransferred,	//완료된 I/O 작업에서 전송된 바이트 수를 받는 변수에 대한 포인터입니다.
  [out] PULONG_PTR   lpCompletionKey,				//I/O 작업이 완료된 파일 핸들과 연결된 완료 키 값을 받는 변수에 대한 포인터입니다.
  [out] LPOVERLAPPED *lpOverlapped,
  [in]  DWORD        dwMilliseconds					//완료 패킷이 완료 포트에 나타날 때까지 호출자가 대기하려는 시간(밀리초)입니다.
);
*/
		DWORD dwTransferredByte = 0;
		ULONG_PTR key = 0;
		Session* pSession = nullptr;
		OVERLAPPED_EX* pOverlappedEx = nullptr;

		//INFINITE > 완료 통지가 올때까지 커널영역 접근 x
		BOOL res = GetQueuedCompletionStatus(m_hIocp,
			&dwTransferredByte,
			&key,
			reinterpret_cast<LPOVERLAPPED*>(&pOverlappedEx),
			INFINITE);

		pSession = reinterpret_cast<Session*>(key);

		//비정상적종료:리턴값 false, 수신바이트 0
		//정상적인종료:리턴값 true, 수신바이트 0
		if (res == false)
		{
			DWORD error = WSAGetLastError();
			switch (error)
			{

				case ERROR_NETNAME_DELETED:
				{
					//클라이언트 hard close 
					//하나하나 파싱해서 종료할것인가???
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
					//서버 종료해야하는 오류?
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

	//클라이언트 소켓을 IOCP에 등록 (비동기 Recv / Send하기위해) /->iocp로이관해야함
	bool res = RegisterIOCP(
		reinterpret_cast<HANDLE>(pSession->GetSocket()), 
		reinterpret_cast<ULONG_PTR>(pSession));

	if (!res)
	{
		SESSION_MANAGER.CloseSession(pSession);
		return;
	}

	pSession->RegisterRecv();  // 여기서 최초 Recv 시작
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
			//잘못된패킷
			HandleDisconnect(pSession->GetSessionID());
			return;
		}

		//아직 다 안왔으면 아무작업 안함!!
		if (iDataSize < header->m_Length)
			break;

		// 완성된 패킷 → OnReceive
		OnReceive(pSession->GetSessionID(), pReadPos, header->m_Length);

		pSession->SuccessRecv(header->m_Length);
	}
	pSession->RegisterRecv(); // 다음 수신 등록

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
	OnDisconnect(iSessionID); // 자식 클래스 콜백
}

