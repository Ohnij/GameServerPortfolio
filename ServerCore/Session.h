
#pragma once
#include "NetDefine.h"
#include <mutex>
#include "RecvBuffer.h"
#include "SendBuffer.h"

//Session
//
// 연결클라이언트 소켓관리 
// recive 버퍼 관리
// recive overlapped확장 관리
//

//상속 안함 무조건 IOCP에서만 사용하고 처리는 IOCP에서 이관하기.
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
	void PrepareSesseion(SOCKET socket); //Bind하면서 초기화

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

	int m_iSesseionID; //세션번호
};