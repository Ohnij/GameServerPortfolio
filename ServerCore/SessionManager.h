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

    //����
    void Init(int iMaxSession);

    //���
    Session* GetEmpty();                    //�󼼼ǰ�������
    Session* Get(int iSessionID);           // �迭 ����
    void CloseSession(Session* pSesseion);         //���� ���� + Ǯ��ȯ

    void ReturnQueue(Session* pSesseion);   //Ǯ��ȯ

private:
    std::mutex m_mutex;

    int m_iMaxSession= 0;
    Session* m_Sessions = nullptr;
    std::queue<Session*> m_EmptyQueue;
};
