#include "stdafx.h"
#include "GameClient.h"
#include "PacketManager.h"

GameClient::GameClient()
{
}

GameClient::~GameClient()
{
}

void GameClient::OnReceive(BYTE* data, int size)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(data);
	

	BYTE* payload = reinterpret_cast<BYTE*>(data + sizeof(PacketHeader));
	PacketManager::Instance().HandlePacket(
		GetSelf(),
		header,
		payload,
		header->packet_length - sizeof(PacketHeader));

}
