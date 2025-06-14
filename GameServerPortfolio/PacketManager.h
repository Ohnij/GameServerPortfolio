#pragma once
#include <functional>
#include <mutex>
#include "NetDefine.h"
#include "jhnet.pb.h"

//패킷매니저가 너무 지저분해서 간결하게 좀 수정할필요있을듯.


struct PacketUnit
{
	int iSessionID;
	std::vector<BYTE> data; // 패킷 전체
};

//매세지 받아서 알아서 처리하는걸로
using PacketHandlerFunction = std::function<bool(int, std::shared_ptr<google::protobuf::Message>, int)>;
#define PACKET_MANAGER PacketManager::Instance()
class PacketManager
{
private:
	PacketManager();
	~PacketManager();

	//패킷핸들러 모음 초기화(insert function)
	void InitPacketHandler();
	template<typename T> //패킷아이디 <=> proto패킷객체연결
	void MakeFactory();	

	template<typename T>
	void MakeHandler(bool (PacketManager::* handler)(int, std::shared_ptr<google::protobuf::Message>, int));
	
	
	

public:
	static PacketManager& Instance();

//서버
	void BindSendFunc(std::function<bool(int, BYTE*, int)> fpSendFunc);
	//직렬화해서 서버 연결된 함수실행 
	//void SendMsg(int iSessionID, const ::google::protobuf::Message& message, PACKET_ID packetID);
	//void SendMsg(int iSessionID, const ::google::protobuf::Message& message);
	template<typename T>
	void SendMsg(int iSessionID, const T& message);
	//서버에서 OnReceive시에 Queue밀어주기.
	void PushQueue(int iSessionID, BYTE* pData, int iSize); 
	

//게임
	//각 스레드에서 이용하여 핸들 패킷을 호출
	bool HandlePacket(int iSessionID, BYTE* pData, int iSize);


private://처리용 변수
	std::mutex m_mutex;
	std::queue<PacketUnit> m_PacketQueue;
	std::function<bool(int, BYTE*, int)> m_SendFunction;
private://맵핑용 변수
	//메시지 생성자 메이커
	std::unordered_map<PACKET_ID, std::function<std::shared_ptr<google::protobuf::Message>()>> m_MesaageFactory;
	//패킷별 함수 핸들러
	std::unordered_map<PACKET_ID, PacketHandlerFunction> m_PacketHandler; // id / GameClient,byte*,int
private:

	bool Handle_CSP_Ping(int iSessionID, std::shared_ptr<google::protobuf::Message> message, int iSize);
	bool Handle_CSP_Echo(int iSessionID, std::shared_ptr<google::protobuf::Message> message, int iSize);
	bool Handle_CSP_Login(int iSessionID, std::shared_ptr<google::protobuf::Message> message, int iSize);
	bool Handle_CSP_CharList(int iSessionID, std::shared_ptr<google::protobuf::Message> message, int iSize);
	bool Handle_CSP_CreateChar(int iSessionID, std::shared_ptr<google::protobuf::Message> message, int iSize);
	bool Handle_CSP_SelectChar(int iSessionID, std::shared_ptr<google::protobuf::Message> message, int iSize);
};
