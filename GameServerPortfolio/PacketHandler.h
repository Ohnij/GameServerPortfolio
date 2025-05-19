#pragma once

#include <unordered_map>
#include <functional>
#include <memory>
#include <vector>
#include "Client.h"
#include "Packet.h"
#include "jhnet.pb.h"


using PacketHandlerFunction = std::function<void(std::shared_ptr<class Client>, BYTE*, int)>;

extern std::unordered_map< PACKET_ID, PacketHandlerFunction> g_PacketHandler; // id / client,byte*,int


//��Ŷ�ڵ鷯 ����(insert function)
void InitPacketHandler();

//����ȭ�κ� (���� �ڵ�) 
void Send(std::shared_ptr<Client> client, const ::google::protobuf::Message& packet, PACKET_ID packet_id);

