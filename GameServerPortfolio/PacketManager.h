#pragma once
#include <functional>
#include "IOCPDefine.h"
#include "jhnet.pb.h"


//using PacketHandlerFunction = std::function<bool(std::shared_ptr<class Client>, BYTE*, int)>;

//�ż��� �޾Ƽ� �˾Ƽ� ó���ϴ°ɷ�
using PacketHandlerFunction = std::function<bool(int, std::shared_ptr<google::protobuf::Message>, int)>;

#define PACKETMANAGER_Send PacketManager::Instance().Send
class PacketManager
{
public:
	static PacketManager& Instance();


	//����ȭ�κ� (���� �ڵ�) (�����ºκ��� Client�� �־ �ȴ�.)
	void Send(int client_id, const ::google::protobuf::Message& packet, PACKET_ID packet_id);
	void Receive(int client_id, BYTE* data, int size);


	//��Ŷ������ ����.
	bool HandlePacket(int client_id, PacketHeader* header, BYTE* data, int size);
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

	void MakeHandler(PACKET_ID id, bool (PacketManager::* handler)(int, std::shared_ptr<google::protobuf::Message> , int))
	{
		_PacketHandler[id] = [this, handler](int client_id, std::shared_ptr<google::protobuf::Message> msg , int size)
		{
			return (this->*handler)(client_id, msg , size);
		};
	}

	//�޽��� ������ ����Ŀ
	std::unordered_map<PACKET_ID, std::function<std::shared_ptr<google::protobuf::Message>()>> _proto_message_factory; //�޼��������� ������?
	//��Ŷ�� �Լ� �ڵ鷯
	std::unordered_map<PACKET_ID, PacketHandlerFunction> _PacketHandler; // id / GameClient,byte*,int

	
private:

	bool Handle_CS_PING(int client_id, std::shared_ptr<google::protobuf::Message> message, int size);
	bool Handle_CS_ECHO(int client_id, std::shared_ptr<google::protobuf::Message> message, int size);
	bool Handle_CS_LOGIN(int client_id, std::shared_ptr<google::protobuf::Message> message, int size);
	bool Handle_CS_CHAR_LIST(int client_id, std::shared_ptr<google::protobuf::Message> message, int size);
	bool Handle_CS_CREATE_CHAR(int client_id, std::shared_ptr<google::protobuf::Message> message, int size);
	bool Handle_CS_SELECT_CHAR(int client_id, std::shared_ptr<google::protobuf::Message> message, int size);
};
