#include "PacketHandler.h"


std::unordered_map<uint16_t, PacketHandlerFunction> g_PacketHandler;

using MessageFactory = std::unordered_map<uint16_t, std::function<::google::protobuf::Message* ()>>;

MessageFactory _message_factory = {
    { jhnet::PacketId::CS_ECHO, []() { return new jhnet::CS_Echo(); } },
};




void Handle_CS_ECHO(std::shared_ptr<Client> client, BYTE* data, int size)
{
    jhnet::CS_Echo packet;
	
    if (!packet.ParseFromArray(data, size)) return;
    std::cout << "echo: number=" << packet.number() << ", message=" << packet.message() << std::endl;
	
    jhnet::SC_Echo res;
    res.set_number(packet.number());
    res.set_message(packet.message());

    Send(client, res, jhnet::PacketId::SC_ECHO);
}

void InitPacketHandler()
{
    g_PacketHandler[jhnet::PacketId::CS_ECHO] = Handle_CS_ECHO;
}



void Send(std::shared_ptr<Client> client, const::google::protobuf::Message& packet, PACKET_ID packet_id)
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

