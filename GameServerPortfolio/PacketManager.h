#pragma once
#include <functional>
#include "Packet.h"
#include "jhnet.pb.h"


//using PacketHandlerFunction = std::function<bool(std::shared_ptr<class Client>, BYTE*, int)>;

//매세지 받아서 알아서 처리하는걸로
using PacketHandlerFunction = std::function<bool(std::shared_ptr<class GameClient>, std::shared_ptr<google::protobuf::Message>)>;

class PacketManager
{
public:
	static PacketManager& Instance();
		
	//직렬화부분 (거의 자동) (보내는부분은 Client만 있어도 된다.)
	void Send(std::shared_ptr<class Client> client, const ::google::protobuf::Message& packet, PACKET_ID packet_id);

	bool HandlePacket(std::shared_ptr<GameClient> client, PacketHeader* header, BYTE* data, int size);
private:
	PacketManager();
	~PacketManager() = default;

	//패킷핸들러 모음 초기화(insert function)
	void InitPacketHandler();


	template<typename T>
	void MakeFactory(PACKET_ID id) 
	{
		_proto_message_factory[id] = []() { return std::make_shared<T>(); };
	}

	void MakeHandler(PACKET_ID id, bool (PacketManager::* handler)(std::shared_ptr<GameClient>, std::shared_ptr<google::protobuf::Message>))
	{
		_PacketHandler[id] = [this, handler](std::shared_ptr<GameClient> client, std::shared_ptr<google::protobuf::Message> msg)
		{
			return (this->*handler)(client, msg);
		};
	}

	//메시지 생성자 메이커
	std::unordered_map<PACKET_ID, std::function<std::shared_ptr<google::protobuf::Message>()>> _proto_message_factory; //메세지생성자 포인터?
	//패킷별 함수 핸들러
	std::unordered_map<PACKET_ID, PacketHandlerFunction> _PacketHandler; // id / GameClient,byte*,int


private:

	bool Handle_CS_PING(std::shared_ptr<GameClient> client, std::shared_ptr<google::protobuf::Message> message);
	bool Handle_CS_ECHO(std::shared_ptr<GameClient> client, std::shared_ptr<google::protobuf::Message> message);
	bool Handle_CS_LOGIN(std::shared_ptr<GameClient> client, std::shared_ptr<google::protobuf::Message> message);
	bool Handle_CS_CHAR_LIST(std::shared_ptr<GameClient> client, std::shared_ptr<google::protobuf::Message> message);
	bool Handle_CS_CREATE_CHAR(std::shared_ptr<GameClient> client, std::shared_ptr<google::protobuf::Message> message);
	bool Handle_CS_SELECT_CHAR(std::shared_ptr<GameClient> client, std::shared_ptr<google::protobuf::Message> message);
};
