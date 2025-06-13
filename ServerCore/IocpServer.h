#pragma once
#include <thread>
#include <mutex>
#include "IOCPDefine.h"


class Listener;
class IocpServer
{
public:
    IocpServer();
    virtual ~IocpServer();

    bool ServerStart(int iMaxClientSession, USHORT port);
    bool SendMsg(int iSessionID, BYTE* pData, int iSize);
private:
    //초기화
    bool InitSocket(USHORT port);
    bool InitIocp();
    void RunWorkerThreads();

    //IOCP등록
    bool RegisterIOCP(HANDLE handle, ULONG_PTR key);

    //워커스레드
    void WorkerThread();

    //이벤트처리
    void HandleAccept(OVERLAPPED_ACCEPT* pOverlappedAccept, DWORD dwTransferredBytes);
    void HandleRecv(OVERLAPPED_RECV* pOverlappedRecv, DWORD dwTransferredBytes);
    void HandleSend(OVERLAPPED_SEND* pOverlappedSend, DWORD dwTransferredBytes);
    void HandleDisconnect(int iSessionID);

private:
    HANDLE m_hIocp = INVALID_HANDLE_VALUE;
    std::shared_ptr<Listener> m_Listener;
    std::vector<std::thread> m_Workers;


public:
    virtual void OnConnect(int iSessionID) = 0;
    virtual void OnReceive(int iSessionID, BYTE* pData, int iSize) = 0;
    virtual void OnSend(int iSessionID, int iSize) = 0;
    virtual void OnDisconnect(int iSessionID) = 0;
};