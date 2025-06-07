#include "stdafx.h"
#include "PacketManager.h"
#include "Packet.h"

//std::unordered_map<uint16_t, PacketHandlerFunction> g_PacketHandler;


void PacketManager::Send(std::shared_ptr<Client> client, const::google::protobuf::Message& packet, PACKET_ID packet_id)
{
	//user , msg, packet_id
	int data_size = packet.ByteSizeLong();
	int packet_size = sizeof(PacketHeader) + data_size;

	//����Ʈ�迭 ����
	std::vector<BYTE> buffer(packet_size);

	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer.data());
	header->packet_length = packet_size;
	header->packet_id = packet_id;

	//jhnet::SC_Echo s;
	//s.SerializeToArray(buffer.data(), packet_size);
	packet.SerializeToArray(buffer.data() + sizeof(PacketHeader), packet_size);
	
	client->RegisterSend(buffer.data(), packet_size);
}

//�ڵ�ȭ�� ����
//PacketHandlerFunction PacketManager::GetPacketHandle(PACKET_ID packet_id)
//{
//	auto func = _PacketHandler.find(packet_id);
//	if (func == _PacketHandler.end())
//		return nullptr;
//
//	return func->second;
//}

bool PacketManager::HandlePacket(std::shared_ptr<Client> client, PacketHeader* header, BYTE* data, int size)
{
	auto factory_iter = _proto_message_factory.find(header->packet_id);
	if (factory_iter == _proto_message_factory.end()) 
	{
		//�ش� ��Ŷ id�� �� �����ھ���
		std::cerr << "�ش� ��Ŷid�� �� �����ڰ� �����ϴ�.. " << header->packet_id << std::endl;
		return false;
	}

	std::shared_ptr<google::protobuf::Message> msg = factory_iter->second();
	if (!msg->ParseFromArray(data, size)) 
	{
		//��Ŷ �Ľ� ����
		std::cerr << "��Ŷ ����ȭ ����.." << header->packet_id << std::endl;
		return false;
	}

	auto func_iter = _PacketHandler.find(header->packet_id);
	if (func_iter == _PacketHandler.end()) 
	{
		//��Ŷ ���� �Լ�������
		std::cerr << "��Ŷ�ڵ鷯 �Լ��������ϴ�.." << header->packet_id << std::endl;
		return false;
	}

	return func_iter->second(client, msg); 
}

PacketManager::PacketManager()
{
	InitPacketHandler();
}


PacketManager& PacketManager::Instance()
{
	static PacketManager _instance;
	return _instance;
}

void PacketManager::InitPacketHandler()
{
	//_PacketHandler[jhnet::PacketId::CS_ECHO] = [this](std::shared_ptr<Client> client, BYTE* data, int size) { return this->Handle_CS_ECHO(client, data, size); };
	_proto_message_factory[jhnet::PacketId::C2S_ECHO] = []() { return std::make_shared<jhnet::CSP_Echo>(); };

	_PacketHandler[jhnet::PacketId::C2S_ECHO] = [this](std::shared_ptr<Client> client, std::shared_ptr<google::protobuf::Message> msg) {
		return this->Handle_CS_ECHO(client, msg);
	};
}

//bool PacketManager::Handle_CS_ECHO(std::shared_ptr<Client> client, BYTE* data, int size)
//{
//	jhnet::CS_Echo packet;
//
//	if (!packet.ParseFromArray(data, size)) return false;
//	std::cout << "["<< GetCurrentThreadId() <<"]client [" << client->GetClientID() << "] echo: number=" << packet.number() << ", message=" << packet.message() << std::endl;
//
//	jhnet::SC_Echo res;
//	res.set_number(packet.number());
//	res.set_message(packet.message());
//
//	Send(client, res, jhnet::PacketId::SC_ECHO);
//}

bool PacketManager::Handle_CS_ECHO(std::shared_ptr<Client> client, std::shared_ptr<google::protobuf::Message> message)
{
	//����Ʈ ������ ����ȯ > static_pointer_cast / dynamic_pointer_cast
	std::shared_ptr<jhnet::CSP_Echo> packet = std::dynamic_pointer_cast<jhnet::CSP_Echo>(message);
	if (!packet)
	{
		//���� �޼����� �ٸ���
		return false;
	}

	std::cout << "[" << GetCurrentThreadId() << "]client [" << client->GetClientID() << "] echo: number=" << packet->number() << ", message=" << packet->message() << std::endl;

	jhnet::SCP_Echo res;
	res.set_number(packet->number());
	res.set_message(packet->message());

	Send(client, res, jhnet::PacketId::S2C_ECHO);
	return true;
}