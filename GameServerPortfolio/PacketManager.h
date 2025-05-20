#pragma once

#include <unordered_map>
#include <functional>
#include <memory>
#include <vector>
#include "Client.h"
#include "Packet.h"
#include "jhnet.pb.h"


using PacketHandlerFunction = std::function<bool(std::shared_ptr<class Client>, BYTE*, int)>;

class PacketManager
{
public:
	static PacketManager& Instance();
		
	//��Ŷ�ڵ鷯 ����(insert function)
	void InitPacketHandler();

	//����ȭ�κ� (���� �ڵ�) 
	void Send(std::shared_ptr<Client> client, const ::google::protobuf::Message& packet, PACKET_ID packet_id);

	PacketHandlerFunction GetPacketHandle(PACKET_ID packet_id);
private:
	std::unordered_map< PACKET_ID, PacketHandlerFunction> _PacketHandler; // id / client,byte*,int
	

	bool Handle_CS_ECHO(std::shared_ptr<Client> client, BYTE* data, int size);
};


