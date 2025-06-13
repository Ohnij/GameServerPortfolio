#pragma once
#include <mutex>

//#include "Client.h"

#define SESSION_MANAGER SessionManager::Instance()
class Session;
class SessionManager
{
private:
    SessionManager() = default;
    ~SessionManager();
public:
    static SessionManager& Instance();

    //생성
    void Init(int iMaxSession);

    //사용
    Session* GetEmpty();                    //빈세션가져오기
    Session* Get(int iSessionID);           // 배열 접근
    void CloseSession(Session* pSesseion);         //연결 종료 + 풀반환

    void ReturnQueue(Session* pSesseion);   //풀반환

private:
    std::mutex m_mutex;

    int m_iMaxSession= 0;
    Session* m_Sessions = nullptr;
    std::queue<Session*> m_EmptyQueue;
};
