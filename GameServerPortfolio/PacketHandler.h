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


//패킷핸들러 모음(insert function)
void InitPacketHandler();

//직렬화부분 (거의 자동) 
void Send(std::shared_ptr<Client> client, const ::google::protobuf::Message& packet, PACKET_ID packet_id);

