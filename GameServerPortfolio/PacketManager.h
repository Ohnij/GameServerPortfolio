#pragma once
#include <functional>
#include "Client.h"
#include "Packet.h"
#include "jhnet.pb.h"


//using PacketHandlerFunction = std::function<bool(std::shared_ptr<class Client>, BYTE*, int)>;

//매세지 받아서 알아서 처리하는걸로
using PacketHandlerFunction = std::function<bool(std::shared_ptr<class Client>, std::shared_ptr<google::protobuf::Message>)>;

class PacketManager
{
public:
	static PacketManager& Instance();
		
	
	

	//직렬화부분 (거의 자동) 
	void Send(std::shared_ptr<Client> client, const ::google::protobuf::Message& packet, PACKET_ID packet_id);

	//PacketHandlerFunction GetPacketHandle(PACKET_ID packet_id);

	bool HandlePacket(std::shared_ptr<Client> client, PacketHeader* header, BYTE* data, int size);
private:
	PacketManager();
	~PacketManager() = default;

	//패킷핸들러 모음(insert function)
	void InitPacketHandler();

	//패킷 메시지 생성자 맵핑
	std::unordered_map<PACKET_ID, std::function<std::shared_ptr<google::protobuf::Message>()>> _proto_message_factory; //메세지생성자 포인터?
	//패킷 메세지별 함수 맵핑
	std::unordered_map<PACKET_ID, PacketHandlerFunction> _PacketHandler; // id / client,byte*,int


	//bool Handle_CS_ECHO(std::shared_ptr<Client> client, BYTE* data, int size);
	bool Handle_CS_ECHO(std::shared_ptr<Client> client, std::shared_ptr<google::protobuf::Message> message);
};


