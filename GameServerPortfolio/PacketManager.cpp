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
	//탬플릿으로 ID매핑
	MakeFactory<jhnet::CSP_Ping>();
	MakeFactory<jhnet::CSP_Echo>();
	MakeFactory<jhnet::CSP_Login>();
	MakeFactory<jhnet::CSP_CharList>();
	MakeFactory<jhnet::CSP_CreateChar>();
	MakeFactory<jhnet::CSP_SelectChar>();

	//탬플릿으로 ID매핑 + 함수명 일치로 실수방지
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
	//패킷을 미리 설정해주어야 ID입력하는 수고를 덜수있다
	//한번 해두면 ID맵핑 실수도 줄어든다
	constexpr uint16_t id = PacketIdMapper<T>::ID;

	//user , msg, packet_id
	int iPayloadSize = message.ByteSizeLong();
	int iPacketSize = sizeof(PacketHeader) + iPayloadSize;

	//바이트배열 생성
	std::vector<BYTE> buffer(iPacketSize);

	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer.data());
	header->length = iPacketSize;
	//header->ID = packetID;
	header->ID = id;

	//직렬화   
	//message -> buffer=[header][message]
	message.SerializeToArray(buffer.data() + sizeof(PacketHeader), iPayloadSize);
	
	if(m_SendFunction)
		m_SendFunction(iSessionID, buffer.data(), iPacketSize);
}

void PacketManager::PushQueue(int iSessionID, BYTE* pData, int iSize)
{
	//서버에서 패킷주면 큐에 넣기
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
		//해당 패킷 id로 된 생성자없음
		std::cerr << "해당 패킷id로 된 생성자가 없습니다.. " << pHeader->ID << std::endl;
		return false;
	}

	//여기서  Make_shared 패킷에 맞는 객체로 받아옴 
	std::shared_ptr<google::protobuf::Message> msg = factory_iter->second(); 
	//여기서 데이터가 복사됨 (data는 지워져도 상관없음 복사가 된상태!!!)
	if (!msg->ParseFromArray(pPayLoad, iPayLoadSize))
	{
		//패킷 파싱 실패
		std::cerr << "패킷 직렬화 실패.." << pHeader->ID << std::endl;
		return false;
	}


	auto func_iter = m_PacketHandler.find(pHeader->ID);
	if (func_iter == m_PacketHandler.end()) 
	{
		//패킷 전용 함수가없다
		std::cerr << "패킷핸들러 함수가없습니다.." << pHeader->ID << std::endl;
		return false;
	}

	return func_iter->second(iSessionID, msg, iPayLoadSize);
}


bool PacketManager::Handle_CSP_Ping(int iSessionID, std::shared_ptr<google::protobuf::Message> message, int iSize)
{
	auto pUser = USER_MANAGER.GetSession(iSessionID);
	if (!pUser)
		return false;

	//스마트 포인터 형변환 > static_pointer_cast / dynamic_pointer_cast
	std::shared_ptr<jhnet::CSP_Ping> packet = std::dynamic_pointer_cast<jhnet::CSP_Ping>(message);
	if (!packet)
	{
		//들어온 메세지가 다르다
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
		//들어온 메세지가 다르다
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
		//들어온 메세지가 다르다
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
		//들어온 메세지가 다르다
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
		//들어온 메세지가 다르다
		return false;
	}

	//캐릭터 생성요청
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
		//들어온 메세지가 다르다
		return false;
	}
	//캐릭터 선택 (입장)
	//uint64 character_uid = 1;
	

	return true;
}
