#include "PacketManager.h"
#include "stdafx.h"
#include "PacketManager.h"
#include <codecvt>
#include "Util.h"

#include "GameServer.h"
#include "PacketIDMapper.h"

#include "UserManager.h"
#include "User.h"
#include "DBManager.h"
#include "DBRequest_CheckAccountLogin.h"
#include "DBRequest_CreateCharacter.h"
#include "DBRequest_GetCharacterList.h"


PacketManager::PacketManager()
{
	InitPacketHandler();
}

PacketManager::~PacketManager()
{

}


void PacketManager::InitPacketHandler()
{
	//���ø����� ID����
	MakeFactory<jhnet::CSP_Ping>();
	MakeFactory<jhnet::CSP_Echo>();
	MakeFactory<jhnet::CSP_Login>();
	MakeFactory<jhnet::CSP_CharList>();
	MakeFactory<jhnet::CSP_CreateChar>();
	MakeFactory<jhnet::CSP_SelectChar>();

	//���ø����� ID���� + �Լ��� ��ġ�� �Ǽ�����
	MakeHandler<jhnet::CSP_Ping>(&PacketManager::Handle_CSP_Ping);
	MakeHandler<jhnet::CSP_Echo>(&PacketManager::Handle_CSP_Echo);
	MakeHandler<jhnet::CSP_Login>(&PacketManager::Handle_CSP_Login);
	MakeHandler<jhnet::CSP_CharList>(&PacketManager::Handle_CSP_CharList);
	MakeHandler<jhnet::CSP_CreateChar>(&PacketManager::Handle_CSP_CreateChar);
	MakeHandler<jhnet::CSP_SelectChar>(&PacketManager::Handle_CSP_SelectChar);
}

template<typename T>
void PacketManager::MakeFactory()
{
	m_MesaageFactory[PacketIdMapper<T>::ID] = []() { return std::make_shared<T>(); };
}

template<typename T>
void PacketManager::MakeHandler(bool(PacketManager::* handler)(int, std::shared_ptr<google::protobuf::Message>, int))
{
	m_PacketHandler[PacketIdMapper<T>::ID] = [this, handler](int client_id, std::shared_ptr<google::protobuf::Message> msg, int size)
	{
		return (this->*handler)(client_id, msg, size);
	};
}

PacketManager& PacketManager::Instance()
{
	static PacketManager _instance;
	return _instance;
}

void PacketManager::BindSendFunc(std::function<bool(int, BYTE*, int)> fpSendFunc)
{
	m_SendFunction = fpSendFunc;
}

template<typename T>
void PacketManager::SendMsg(int iSessionID, const T& message)
{
	//��Ŷ�� �̸� �������־�� ID�Է��ϴ� ���� �����ִ�
	//�ѹ� �صθ� ID���� �Ǽ��� �پ���
	constexpr uint16_t id = PacketIdMapper<T>::ID;

	//user , msg, packet_id
	int iPayloadSize = message.ByteSizeLong();
	int iPacketSize = sizeof(PacketHeader) + iPayloadSize;

	//����Ʈ�迭 ����
	std::vector<BYTE> buffer(iPacketSize);

	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer.data());
	header->length = iPacketSize;
	//header->ID = packetID;
	header->ID = id;

	//����ȭ   
	//message -> buffer=[header][message]
	message.SerializeToArray(buffer.data() + sizeof(PacketHeader), iPayloadSize);
	
	if(m_SendFunction)
		m_SendFunction(iSessionID, buffer.data(), iPacketSize);
}

void PacketManager::PushQueue(int iSessionID, BYTE* pData, int iSize)
{
	//�������� ��Ŷ�ָ� ť�� �ֱ�
	std::lock_guard<std::mutex> lock(m_mutex);

	PacketUnit pkt;
	pkt.iSessionID = iSessionID;
	pkt.data.resize(iSize);
	memcpy(pkt.data.data(), pData, iSize);

	m_PacketQueue.push(std::move(pkt));
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
//////								
//////////////////////////////////////////////////////////////////////////////////////////////////////


bool PacketManager::HandlePacket(int iSessionID, BYTE* pData, int iSize)
{

	PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(pData);
	BYTE* pPayLoad = pData + sizeof(PacketHeader);
	int iPayLoadSize = pHeader->length - sizeof(PacketHeader);

	auto factory_iter = m_MesaageFactory.find(pHeader->ID);
	if (factory_iter == m_MesaageFactory.end()) 
	{
		//�ش� ��Ŷ id�� �� �����ھ���
		std::cerr << "�ش� ��Ŷid�� �� �����ڰ� �����ϴ�.. " << pHeader->ID << std::endl;
		return false;
	}

	//���⼭  Make_shared ��Ŷ�� �´� ��ü�� �޾ƿ� 
	std::shared_ptr<google::protobuf::Message> msg = factory_iter->second(); 
	//���⼭ �����Ͱ� ����� (data�� �������� ������� ���簡 �Ȼ���!!!)
	if (!msg->ParseFromArray(pPayLoad, iPayLoadSize))
	{
		//��Ŷ �Ľ� ����
		std::cerr << "��Ŷ ����ȭ ����.." << pHeader->ID << std::endl;
		return false;
	}


	auto func_iter = m_PacketHandler.find(pHeader->ID);
	if (func_iter == m_PacketHandler.end()) 
	{
		//��Ŷ ���� �Լ�������
		std::cerr << "��Ŷ�ڵ鷯 �Լ��������ϴ�.." << pHeader->ID << std::endl;
		return false;
	}

	return func_iter->second(iSessionID, msg, iPayLoadSize);
}


bool PacketManager::Handle_CSP_Ping(int iSessionID, std::shared_ptr<google::protobuf::Message> message, int iSize)
{
	auto pUser = USER_MANAGER.GetSession(iSessionID);
	if (!pUser)
		return false;

	//����Ʈ ������ ����ȯ > static_pointer_cast / dynamic_pointer_cast
	std::shared_ptr<jhnet::CSP_Ping> packet = std::dynamic_pointer_cast<jhnet::CSP_Ping>(message);
	if (!packet)
	{
		//���� �޼����� �ٸ���
		return false;
	}

	std::cout << "\
[Thread " << GetCurrentThreadId() << "]\
[Session " << iSessionID << "]\
[Packet CSP_Ping]\
number=" << packet->number() << ",\
timstamp=" << packet->timestamp() << "\n";

	jhnet::CSP_Ping res;
	res.set_number(packet->number());
	res.set_timestamp(0);

	SendMsg(iSessionID, res);
	return true;
}


bool PacketManager::Handle_CSP_Echo(int iSessionID, std::shared_ptr<google::protobuf::Message> message, int iSize)
{
	auto pUser = USER_MANAGER.GetSession(iSessionID);
	if (!pUser)
		return false;

	std::shared_ptr<jhnet::CSP_Echo> packet = std::dynamic_pointer_cast<jhnet::CSP_Echo>(message);
	if (!packet)
	{
		//���� �޼����� �ٸ���
		return false;
	}

	std::cout << "\
[Thread " << GetCurrentThreadId() << "]\
[Session " << iSessionID << "]\
[Packet CSP_Echo]\
number=" << packet->number() << ",\
message=" << packet->message() << "\n";

	jhnet::SCP_Echo res;
	res.set_number(packet->number());
	res.set_message(packet->message());

	SendMsg(iSessionID, res);
	return true;
}



bool PacketManager::Handle_CSP_Login(int iSessionID, std::shared_ptr<google::protobuf::Message> message, int iSize)
{
	auto pUser = USER_MANAGER.GetSession(iSessionID);
	if (!pUser)
		return false;

	std::shared_ptr<jhnet::CSP_Login> packet = std::dynamic_pointer_cast<jhnet::CSP_Login>(message);
	if (!packet)
	{
		//���� �޼����� �ٸ���
		return false;
	}

	DBManager::Instance().PushRequest(std::make_shared<DBRequest_CheckAccountLogin>(packet->login_id(), packet->login_pw(), iSessionID));
	return true;
}



bool PacketManager::Handle_CSP_CharList(int iSessionID, std::shared_ptr<google::protobuf::Message> message, int iSize)
{
	auto pUser = USER_MANAGER.GetSession(iSessionID);
	if (!pUser)
		return false;

	std::shared_ptr<jhnet::CSP_CharList> packet = std::dynamic_pointer_cast<jhnet::CSP_CharList>(message);
	if (!packet)
	{
		//���� �޼����� �ٸ���
		return false;
	}

	DBManager::Instance().PushRequest(std::make_shared<DBRequest_GetCharacterList>(pUser->GetAccountID(), iSessionID));
	return true;
}

bool PacketManager::Handle_CSP_CreateChar(int iSessionID, std::shared_ptr<google::protobuf::Message> message, int iSize)
{
	auto pUser = USER_MANAGER.GetSession(iSessionID);
	if (!pUser)
		return false;

	std::shared_ptr<jhnet::CSP_CreateChar> packet = std::dynamic_pointer_cast<jhnet::CSP_CreateChar>(message);
	if (!packet)
	{
		//���� �޼����� �ٸ���
		return false;
	}

	//ĳ���� ������û
	//string name = 1;
	//int32 job_code = 2;

	std::wstring nickname = Utf8ToWString(packet->name());
	DBManager::Instance().PushRequest(std::make_shared<DBRequest_CreateCharacter>(nickname, pUser->GetAccountID(), packet->job_code(), iSessionID));
	return true;
}

bool PacketManager::Handle_CSP_SelectChar(int iSessionID, std::shared_ptr<google::protobuf::Message> message, int iSize)
{
	auto pUser = USER_MANAGER.GetSession(iSessionID);
	if (!pUser)
		return false;

	std::shared_ptr<jhnet::CSP_SelectChar> packet = std::dynamic_pointer_cast<jhnet::CSP_SelectChar>(message);
	if (!packet)
	{
		//���� �޼����� �ٸ���
		return false;
	}
	//ĳ���� ���� (����)
	//uint64 character_uid = 1;
	

	return true;
}
