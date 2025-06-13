#include "pch.h"
#include "SessionManager.h"
#include "Session.h"




SessionManager::~SessionManager()
{
	if (nullptr != m_Sessions)
		delete[] m_Sessions;
}

SessionManager& SessionManager::Instance()
{
	static SessionManager instance; 
	return instance;
}

void SessionManager::Init(int iMaxSession)
{
	m_iMaxSession = iMaxSession;
	m_Sessions = new Session[iMaxSession];
	for (int i = 0; i < iMaxSession; ++i)
	{
		m_Sessions[i].Init(i);
		m_EmptyQueue.push(&m_Sessions[i]);
	}
}


Session* SessionManager::GetEmpty()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	
	if (m_EmptyQueue.empty())
		return nullptr;

	Session* session = m_EmptyQueue.front();
	m_EmptyQueue.pop();

	return session;
}



Session* SessionManager::Get(int client_id)
{
	if (client_id < 0 || m_iMaxSession <= client_id)
		return nullptr;

	return &m_Sessions[client_id];
}

void SessionManager::CloseSession(Session* pSesseion)
{
	if (nullptr == pSesseion)
		return;
	if (pSesseion->IsConnected() == false) //이미 종료되어있음
		return;
	pSesseion->Close();
	ReturnQueue(pSesseion);
}

void SessionManager::ReturnQueue(Session* pSesseion)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_EmptyQueue.push(pSesseion);
}