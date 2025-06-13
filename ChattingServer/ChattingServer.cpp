#include "stdfx.h"
#include "ChattingServer.h"
#include "PacketManager.h"

bool ChattingServer::Start(USHORT port)
{
	PacketManager::Instance().Init([this](int iSession, BYTE* pData, int iSize) {
		return SendMsg(iSession,pData, iSize);
	});
	//서버별로 설정하기.
	return ServerStart(100, port);
}

void ChattingServer::OnConnect(int iSessionID)
{
    std::cerr << "[Connect]" << iSessionID << " connected\n";
}

void ChattingServer::OnReceive(int iSessionID, BYTE* pData, int iSize)
{
	//std::cerr << "[Recv]" << iSessionID << ": " << msg << "\n";
	PacketManager::Instance().ParsePacket(iSessionID, pData, iSize);
}

void ChattingServer::OnSend(int iSessionID, int iSize)
{
	std::cerr << "[Send]" << iSessionID << ": " << iSize << " bytes\n";
}

void ChattingServer::OnDisconnect(int iSessionID)
{
	std::cerr << "[Disconnect] " << iSessionID << " disconnected\n";
}
