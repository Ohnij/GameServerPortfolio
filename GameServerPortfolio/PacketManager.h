#pragma once
#include <functional>
#include "IOCPDefine.h"
#include "jhnet.pb.h"


//using PacketHandlerFunction = std::function<bool(std::shared_ptr<class Client>, BYTE*, int)>;

//매세지 받아서 알아서 처리하는걸로
using PacketHandlerFunction = std::function<bool(int, std::shared_ptr<google::protobuf::Message>, int)>;

#define PACKETMANAGER_Send PacketManager::Instance().Send
class PacketManager
{
public:
	static PacketManager& Instance();


	//직렬화부분 (거의 자동) (보내는부분은 Client만 있어도 된다.)
	void Send(int client_id, const ::google::protobuf::Message& packet, PACKET_ID packet_id);
	void Receive(int client_id, BYTE* data, int size);


	//패킷꺼내서 사용시.
	bool HandlePacket(int client_id, PacketHeader* header, BYTE* data, int size);
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

	void MakeHandler(PACKET_ID id, bool (PacketManager::* handler)(int, std::shared_ptr<google::protobuf::Message> , int))
	{
		_PacketHandler[id] = [this, handler](int client_id, std::shared_ptr<google::protobuf::Message> msg , int size)
		{
			return (this->*handler)(client_id, msg , size);
		};
	}

	//메시지 생성자 메이커
	std::unordered_map<PACKET_ID, std::function<std::shared_ptr<google::protobuf::Message>()>> _proto_message_factory; //메세지생성자 포인터?
	//패킷별 함수 핸들러
	std::unordered_map<PACKET_ID, PacketHandlerFunction> _PacketHandler; // id / GameClient,byte*,int

	
private:

	bool Handle_CS_PING(int client_id, std::shared_ptr<google::protobuf::Message> message, int size);
	bool Handle_CS_ECHO(int client_id, std::shared_ptr<google::protobuf::Message> message, int size);
	bool Handle_CS_LOGIN(int client_id, std::shared_ptr<google::protobuf::Message> message, int size);
	bool Handle_CS_CHAR_LIST(int client_id, std::shared_ptr<google::protobuf::Message> message, int size);
	bool Handle_CS_CREATE_CHAR(int client_id, std::shared_ptr<google::protobuf::Message> message, int size);
	bool Handle_CS_SELECT_CHAR(int client_id, std::shared_ptr<google::protobuf::Message> message, int size);
};
