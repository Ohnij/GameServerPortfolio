#include "PacketManager.h"
#include "stdafx.h"
#include "PacketManager.h"
#include <codecvt>
#include "Util.h"

#include "Packet.h"

#include "User.h"
#include "DBManager.h"
#include "DBRequest_CheckAccountLogin.h"
#include "DBRequest_CreateCharacter.h"
#include "DBRequest_GetCharacterList.h"


void PacketManager::Send(int client_id, const::google::protobuf::Message& packet, PACKET_ID packet_id)
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
	

	Server에서 send (client_id, buffer.data(), packet_size);
	
}

void PacketManager::Receive(int client_id, BYTE* data, int size)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(data);


	BYTE* payload = reinterpret_cast<BYTE*>(data + sizeof(PacketHeader));

	auto factory_iter = _proto_message_factory.find(header->packet_id);
	if (factory_iter == _proto_message_factory.end())
	{
		//해당 패킷 id로 된 생성자없음
		std::cerr << "해당 패킷id로 된 생성자가 없습니다.. " << header->packet_id << std::endl;
		return;
	}




}

//자동화로 삭제
//PacketHandlerFunction PacketManager::GetPacketHandle(PACKET_ID packet_id)
//{
//	auto func = _PacketHandler.find(packet_id);
//	if (func == _PacketHandler.end())
//		return nullptr;
//
//	return func->second;
//}

bool PacketManager::HandlePacket(int client_id, PacketHeader* header, BYTE* data, int size)
{
	auto factory_iter = _proto_message_factory.find(header->packet_id);
	if (factory_iter == _proto_message_factory.end()) 
	{
		//해당 패킷 id로 된 생성자없음
		std::cerr << "해당 패킷id로 된 생성자가 없습니다.. " << header->packet_id << std::endl;
		return false;
	}

	//여기서  Make_shared 패킷에 맞는 객체로 받아옴 
	std::shared_ptr<google::protobuf::Message> msg = factory_iter->second(); 
	//여기서 데이터가 복사됨 (data는 지워져도 상관없음 복사가 된상태!!!)
	if (!msg->ParseFromArray(data, size))  
	{
		//패킷 파싱 실패
		std::cerr << "패킷 직렬화 실패.." << header->packet_id << std::endl;
		return false;
	}


	auto func_iter = _PacketHandler.find(header->packet_id);
	if (func_iter == _PacketHandler.end()) 
	{
		//패킷 전용 함수가없다
		std::cerr << "패킷핸들러 함수가없습니다.." << header->packet_id << std::endl;
		return false;
	}

	return func_iter->second(client_id, msg, size);
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
	MakeFactory<jhnet::CSP_Ping>(jhnet::PacketId::C2S_PING);
	MakeFactory<jhnet::CSP_Echo>(jhnet::PacketId::C2S_ECHO);
	MakeFactory<jhnet::CSP_Login>(jhnet::PacketId::C2S_LOGIN);
	MakeFactory<jhnet::CSP_CharList>(jhnet::PacketId::C2S_CHAR_LIST);
	MakeFactory<jhnet::CSP_CreateChar>(jhnet::PacketId::C2S_CREATE_CHAR);
	MakeFactory<jhnet::CSP_SelectChar>(jhnet::PacketId::C2S_SELECT_CHAR);


	MakeHandler(jhnet::PacketId::C2S_PING , &PacketManager::Handle_CS_PING);
	MakeHandler(jhnet::PacketId::C2S_ECHO, &PacketManager::Handle_CS_ECHO);
	MakeHandler(jhnet::PacketId::C2S_LOGIN, &PacketManager::Handle_CS_LOGIN);
	MakeHandler(jhnet::PacketId::C2S_CHAR_LIST, &PacketManager::Handle_CS_CHAR_LIST);
	MakeHandler(jhnet::PacketId::C2S_CREATE_CHAR, &PacketManager::Handle_CS_CREATE_CHAR);
	MakeHandler(jhnet::PacketId::C2S_SELECT_CHAR, &PacketManager::Handle_CS_SELECT_CHAR);
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

bool PacketManager::Handle_CS_PING(int client_id, std::shared_ptr<google::protobuf::Message> message, int size )
{
	//스마트 포인터 형변환 > static_pointer_cast / dynamic_pointer_cast
	

	std::shared_ptr<jhnet::CSP_Ping> packet = std::dynamic_pointer_cast<jhnet::CSP_Ping>(message);
	if (!packet)
	{
		//들어온 메세지가 다르다
		return false;
	}

	std::cout << "[" << GetCurrentThreadId() << "]client [" << client->GetClientID() << "] ping: number=" << packet->number() << ", timstamp=" << packet->timestamp() << "\n";

	jhnet::CSP_Ping res;
	res.set_number(packet->number());
	res.set_timestamp(0);

	Send(client, res, jhnet::PacketId::S2C_PING);
	return true;
}


bool PacketManager::Handle_CS_ECHO(int client_id, std::shared_ptr<google::protobuf::Message> message, int size)
{
	//스마트 포인터 형변환 > static_pointer_cast / dynamic_pointer_cast
	std::shared_ptr<jhnet::CSP_Echo> packet = std::dynamic_pointer_cast<jhnet::CSP_Echo>(message);
	if (!packet)
	{
		//들어온 메세지가 다르다
		return false;
	}

	std::cout << "[" << GetCurrentThreadId() << "]client [" << client->GetClientID() << "] echo: number=" << packet->number() << ", message=" << packet->message() << "\n";

	jhnet::SCP_Echo res;
	res.set_number(packet->number());
	res.set_message(packet->message());

	Send(client, res, jhnet::PacketId::S2C_ECHO);
	return true;
}

bool PacketManager::Handle_CS_LOGIN(int client_id, std::shared_ptr<google::protobuf::Message> message, int size)
{
	std::shared_ptr<jhnet::CSP_Login> packet = std::dynamic_pointer_cast<jhnet::CSP_Login>(message);
	if (!packet)
	{
		//들어온 메세지가 다르다
		return false;
	}

	DBManager::Instance().PushRequest(std::make_shared<DBRequest_CheckAccountLogin>(packet->login_id(), packet->login_pw(), client));
	return true;
}

bool PacketManager::Handle_CS_CHAR_LIST(int client_id, std::shared_ptr<google::protobuf::Message> message, int size)
{
	std::shared_ptr<jhnet::CSP_CharList> packet = std::dynamic_pointer_cast<jhnet::CSP_CharList>(message);
	if (!packet)
	{
		//들어온 메세지가 다르다
		return false;
	}

	DBManager::Instance().PushRequest(std::make_shared<DBRequest_GetCharacterList>(client->GetAccountUid(), client));
	return true;
}

bool PacketManager::Handle_CS_CREATE_CHAR(int client_id, std::shared_ptr<google::protobuf::Message> message, int size)
{
	std::shared_ptr<jhnet::CSP_CreateChar> packet = std::dynamic_pointer_cast<jhnet::CSP_CreateChar>(message);
	if (!packet)
	{
		//들어온 메세지가 다르다
		return false;
	}

	//캐릭터 생성요청
	//string name = 1;
	//int32 job_code = 2;

	std::wstring nickname = Utf8ToWString(packet->name());
	DBManager::Instance().PushRequest(std::make_shared<DBRequest_CreateCharacter>(nickname, client->GetAccountUid(), packet->job_code(), client));
	return true;
}

bool PacketManager::Handle_CS_SELECT_CHAR(int client_id, std::shared_ptr<google::protobuf::Message> message, int size)
{
	std::shared_ptr<jhnet::CSP_SelectChar> packet = std::dynamic_pointer_cast<jhnet::CSP_SelectChar>(message);
	if (!packet)
	{
		//들어온 메세지가 다르다
		return false;
	}
	//캐릭터 선택 (입장)
	//uint64 character_uid = 1;
	

	return true;
}
