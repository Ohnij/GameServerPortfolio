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
	//������Ʈ Ǯ�ݳ��� Manager���� �����Ұ�
	if (m_Socket != INVALID_SOCKET)
	{
		CancelIoEx(reinterpret_cast<HANDLE>(m_Socket), nullptr);
		shutdown(m_Socket, SD_BOTH); //���� �ش� ���� �ۼ��� ����!!
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
	//�׽�Ʈ �غ��ϱ� flag���� �ȳְ� ��� 0���� ������ nullptr������ ������..
	WSARecv(m_Socket, &wsabuf, 1, &recvByte, &flag, static_cast<OVERLAPPED*>(&m_OverlappedRecv), nullptr);
}

bool Session::RegisterSend(BYTE* pData, int iSize)
{
	//Game���� ������ IOCP X

	if (!m_SendBuffer.Write(pData, iSize))
	{
		//log?
		return false; //���� ���н� �׳� false���� (�뷮����)
	}

	if (!m_bSending.exchange(true)) // ���� ���� �ƴϸ� true�� ���� (�̹��������̸� ����� !true�� ������ �ȵ���)
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
	//IOCP���� 
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
			//?������Ȳ?
			return false;
		}
	}
	return true;
}

