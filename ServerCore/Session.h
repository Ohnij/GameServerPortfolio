
#pragma once
#include "NetDefine.h"
#include <mutex>
#include "RecvBuffer.h"
#include "SendBuffer.h"

//Session
//
// ����Ŭ���̾�Ʈ ���ϰ��� 
// recive ���� ����
// recive overlappedȮ�� ����
//

//��� ���� ������ IOCP������ ����ϰ� ó���� IOCP���� �̰��ϱ�.
class Session
{
public:
	Session();
	~Session();

public:
	inline bool IsConnected() { return m_bIsCoonected.load(); }
	inline int GetSessionID() { return m_iSesseionID; }
	inline SOCKET GetSocket() { return m_Socket; }

	inline const RecvBuffer& GetRecvBuffer() const { return m_RecvBuffer; }
public:
	void Init(int iSesseion);
	void PrepareSesseion(SOCKET socket); //Bind�ϸ鼭 �ʱ�ȭ

	void Close();

	void RegisterRecv();
	bool RegisterSend(BYTE* pData, int iSize);

	void ProcessRecv(const int iRecvBytes);
	void ProcessSend(const int iSendBytes);
	
	void SuccessRecv(const int iRecvBytes);
private:
	bool Send();
private:
	std::atomic<bool> m_bIsCoonected = false;
	SOCKET m_Socket = INVALID_SOCKET;

	OVERLAPPED_RECV m_OverlappedRecv;
	OVERLAPPED_SEND m_OverlappedSend;

	RecvBuffer m_RecvBuffer;
	SendBuffer m_SendBuffer;

	std::mutex m_mutex;
	std::atomic<bool> m_bSending = false;

	int m_iSesseionID; //���ǹ�ȣ
};