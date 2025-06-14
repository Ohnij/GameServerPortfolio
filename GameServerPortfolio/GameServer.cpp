#include "stdafx.h"
#include "GameServer.h"
#include "PacketManager.h"


bool GameServer::Start(USHORT port)
{
	PACKET_MANAGER.BindSendFunc([this](int iSession, BYTE* pData, int iSize) {
		return SendMsg(iSession, pData, iSize);
	});
	
	//서버별로 설정하기.
	return ServerStart(3000, port);
}


void GameServer::OnConnect(int iSessionID)
{
	std::cerr << "[Connect]" << iSessionID << " connected\n";
}

void GameServer::OnReceive(int iSessionID, BYTE* pData, int iSize)
{
	std::cerr << "[Recv]" << iSessionID << ": (" << iSize << " bytes)\n";

	PACKET_MANAGER.PushQueue(iSessionID, pData, iSize);
}

void GameServer::OnSend(int iSessionID, int iSize)
{
	std::cerr << "[Send]" << iSessionID << ": (" << iSize << " bytes)\n";
}

void GameServer::OnDisconnect(int iSessionID)
{
	std::cerr << "[Disconnect] " << iSessionID << " disconnected\n";
}

