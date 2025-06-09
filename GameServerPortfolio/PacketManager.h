#pragma once
#include <functional>
#include "Packet.h"
#include "jhnet.pb.h"


//using PacketHandlerFunction = std::function<bool(std::shared_ptr<class Client>, BYTE*, int)>;

//�ż��� �޾Ƽ� �˾Ƽ� ó���ϴ°ɷ�
using PacketHandlerFunction = std::function<bool(std::shared_ptr<class GameClient>, std::shared_ptr<google::protobuf::Message>)>;

class PacketManager
{
public:
	static PacketManager& Instance();
		
	//����ȭ�κ� (���� �ڵ�) (�����ºκ��� Client�� �־ �ȴ�.)
	void Send(std::shared_ptr<class Client> client, const ::google::protobuf::Message& packet, PACKET_ID packet_id);

	bool HandlePacket(std::shared_ptr<GameClient> client, PacketHeader* header, BYTE* data, int size);
private:
	PacketManager();
	~PacketManager() = default;

	//��Ŷ�ڵ鷯 ���� �ʱ�ȭ(insert function)
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

	//�޽��� ������ ����Ŀ
	std::unordered_map<PACKET_ID, std::function<std::shared_ptr<google::protobuf::Message>()>> _proto_message_factory; //�޼��������� ������?
	//��Ŷ�� �Լ� �ڵ鷯
	std::unordered_map<PACKET_ID, PacketHandlerFunction> _PacketHandler; // id / GameClient,byte*,int


private:

	bool Handle_CS_PING(std::shared_ptr<GameClient> client, std::shared_ptr<google::protobuf::Message> message);
	bool Handle_CS_ECHO(std::shared_ptr<GameClient> client, std::shared_ptr<google::protobuf::Message> message);
	bool Handle_CS_LOGIN(std::shared_ptr<GameClient> client, std::shared_ptr<google::protobuf::Message> message);
	bool Handle_CS_CHAR_LIST(std::shared_ptr<GameClient> client, std::shared_ptr<google::protobuf::Message> message);
	bool Handle_CS_CREATE_CHAR(std::shared_ptr<GameClient> client, std::shared_ptr<google::protobuf::Message> message);
	bool Handle_CS_SELECT_CHAR(std::shared_ptr<GameClient> client, std::shared_ptr<google::protobuf::Message> message);
};
