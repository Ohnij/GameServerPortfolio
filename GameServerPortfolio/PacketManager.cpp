#include "PacketManager.h"
#include "Packet.h"

//std::unordered_map<uint16_t, PacketHandlerFunction> g_PacketHandler;


void PacketManager::Send(std::shared_ptr<Client> client, const::google::protobuf::Message& packet, PACKET_ID packet_id)
{
	//user , msg, packet_id
	int data_size = packet.ByteSizeLong();
	int packet_size = sizeof(PacketHeader) + data_size;

	//바이트배열 생성
	std::vector<BYTE> buffer(packet_size);

	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer.data());
	header->packet_length = packet_size;
	header->packet_id = packet_id;

	//jhnet::SC_Echo s;
	//s.SerializeToArray(buffer.data(), packet_size);
	packet.SerializeToArray(buffer.data() + sizeof(PacketHeader), packet_size);

	client->SendPacket(buffer.data(), packet_size);
}

PacketHandlerFunction PacketManager::GetPacketHandle(PACKET_ID packet_id)
{
	auto func = _PacketHandler.find(packet_id);
	if (func == _PacketHandler.end())
		return nullptr;

	return func->second;
}

PacketManager& PacketManager::Instance()
{
	static PacketManager _instance;
	return _instance;
}

void PacketManager::InitPacketHandler()
{
	_PacketHandler[jhnet::PacketId::CS_ECHO] = [this](std::shared_ptr<Client> client, BYTE* data, int size) { return this->Handle_CS_ECHO(client, data, size); };
}

bool PacketManager::Handle_CS_ECHO(std::shared_ptr<Client> client, BYTE* data, int size)
{
	jhnet::CS_Echo packet;

	if (!packet.ParseFromArray(data, size)) return false;
	std::cout << "client [" << client->GetClientID() << "] echo: number=" << packet.number() << ", message=" << packet.message() << std::endl;

	jhnet::SC_Echo res;
	res.set_number(packet.number());
	res.set_message(packet.message());

	Send(client, res, jhnet::PacketId::SC_ECHO);
}