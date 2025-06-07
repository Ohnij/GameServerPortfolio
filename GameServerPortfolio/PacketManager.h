#pragma once
#include <functional>
#include "Client.h"
#include "Packet.h"
#include "jhnet.pb.h"


//using PacketHandlerFunction = std::function<bool(std::shared_ptr<class Client>, BYTE*, int)>;

//�ż��� �޾Ƽ� �˾Ƽ� ó���ϴ°ɷ�
using PacketHandlerFunction = std::function<bool(std::shared_ptr<class Client>, std::shared_ptr<google::protobuf::Message>)>;

class PacketManager
{
public:
	static PacketManager& Instance();
		
	
	

	//����ȭ�κ� (���� �ڵ�) 
	void Send(std::shared_ptr<Client> client, const ::google::protobuf::Message& packet, PACKET_ID packet_id);

	//PacketHandlerFunction GetPacketHandle(PACKET_ID packet_id);

	bool HandlePacket(std::shared_ptr<Client> client, PacketHeader* header, BYTE* data, int size);
private:
	PacketManager();
	~PacketManager() = default;

	//��Ŷ�ڵ鷯 ����(insert function)
	void InitPacketHandler();

	//��Ŷ �޽��� ������ ����
	std::unordered_map<PACKET_ID, std::function<std::shared_ptr<google::protobuf::Message>()>> _proto_message_factory; //�޼��������� ������?
	//��Ŷ �޼����� �Լ� ����
	std::unordered_map<PACKET_ID, PacketHandlerFunction> _PacketHandler; // id / client,byte*,int


	//bool Handle_CS_ECHO(std::shared_ptr<Client> client, BYTE* data, int size);
	bool Handle_CS_ECHO(std::shared_ptr<Client> client, std::shared_ptr<google::protobuf::Message> message);
};


