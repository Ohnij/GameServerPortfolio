#include "pch.h"
#include "Session.h"

Session::Session()
{
}

Session::~Session()
{
}

void Session::Init(int iSesseionID)
{
	m_iSesseionID = iSesseionID;

}

void Session::PrepareSesseion(SOCKET socket)
{
	m_Socket = socket;
	
	ZeroMemory(&m_OverlappedRecv, sizeof(OVERLAPPED_RECV));
	ZeroMemory(&m_OverlappedSend, sizeof(OVERLAPPED_SEND));
	
	m_RecvBuffer.Reset();
	m_SendBuffer.Reset();
	//_recive.socket = client_socket;
	m_OverlappedRecv.m_iSessionID = GetSessionID();
	m_OverlappedRecv.type = IOCP_WORK::IOCP_RECV;
	m_OverlappedSend.m_iSessionID = GetSessionID();
	m_OverlappedSend.type = IOCP_WORK::IOCP_SEND;
	
	m_bSending.store(false);
	m_bIsCoonected.store(true);
}

void Session::Close()
{
	//오브젝트 풀반납시 Manager에서 실행할것
	if (m_Socket != INVALID_SOCKET)
	{
		CancelIoEx(reinterpret_cast<HANDLE>(m_Socket), nullptr);
		shutdown(m_Socket, SD_BOTH); //이제 해당 소켓 송수신 안함!!
		closesocket(m_Socket);
	}
	m_Socket = INVALID_SOCKET;
	m_bIsCoonected.store(false);
}

void Session::RegisterRecv()
{
	m_OverlappedRecv.Reset();

	WSABUF wsabuf;
	wsabuf.buf = reinterpret_cast<CHAR*>(m_RecvBuffer.GetWritePos());
	wsabuf.len = m_RecvBuffer.FreeSize();

	DWORD recvByte = 0;
	DWORD flag = 0;
	//테스트 해보니까 flag변수 안넣고 상수 0으로 넣으면 nullptr참조로 에러남..
	WSARecv(m_Socket, &wsabuf, 1, &recvByte, &flag, static_cast<OVERLAPPED*>(&m_OverlappedRecv), nullptr);
}

bool Session::RegisterSend(BYTE* pData, int iSize)
{
	//Game에서 삽입함 IOCP X

	if (!m_SendBuffer.Write(pData, iSize))
	{
		//log?
		return false; //쓰기 실패시 그냥 false리턴 (용량부족)
	}

	if (!m_bSending.exchange(true)) // 전송 중이 아니면 true로 설정 (이미전송중이면 결과가 !true라 안으로 안들어옴)
	{
		Send();
	}
	return true;
}

void Session::ProcessRecv(int iRecvBytes)
{
	m_RecvBuffer.Write(iRecvBytes);
}

void Session::ProcessSend(const int iSendBytes)
{
	//IOCP에서 
	m_SendBuffer.Read(iSendBytes);

	if (m_SendBuffer.GetDataSize() > 0)
	{
		Send();
	}
	else
	{
		m_bSending.store(false);
	}
}

void Session::SuccessRecv(const int iRecvBytes)
{
	m_RecvBuffer.Read(iRecvBytes);
}


bool Session::Send() //private
{
	WSABUF wsabuf = m_SendBuffer.GetWSABUF();
	m_OverlappedSend.Reset();

	DWORD sendByte = 0;
	DWORD flag = 0;
	int ret = WSASend(m_Socket, &wsabuf, 1, &sendByte, flag,
		reinterpret_cast<OVERLAPPED*>(&m_OverlappedSend), nullptr);

	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != WSA_IO_PENDING)
		{
			m_bSending.store(false);
			//?오류상황?
			return false;
		}
	}
	return true;
}

