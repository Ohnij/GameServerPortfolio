#pragma once
#include <functional>
#include <mutex>
#include "NetDefine.h"
#include "jhnet.pb.h"

//��Ŷ�Ŵ����� �ʹ� �������ؼ� �����ϰ� �� �������ʿ�������.


struct PacketUnit
{
	int iSessionID;
	std::vector<BYTE> data; // ��Ŷ ��ü
};

//�ż��� �޾Ƽ� �˾Ƽ� ó���ϴ°ɷ�
using PacketHandlerFunction = std::function<bool(int, std::shared_ptr<google::protobuf::Message>, int)>;
#define PACKET_MANAGER PacketManager::Instance()
class PacketManager
{
private:
	PacketManager();
	~PacketManager();

	//��Ŷ�ڵ鷯 ���� �ʱ�ȭ(insert function)
	void InitPacketHandler();
	template<typename T> //��Ŷ���̵� <=> proto��Ŷ��ü����
	void MakeFactory();	

	template<typename T>
	void MakeHandler(bool (PacketManager::* handler)(int, std::shared_ptr<google::protobuf::Message>, int));
	
	
	

public:
	static PacketManager& Instance();

//����
	void BindSendFunc(std::function<bool(int, BYTE*, int)> fpSendFunc);
	//����ȭ�ؼ� ���� ����� �Լ����� 
	//void SendMsg(int iSessionID, const ::google::protobuf::Message& message, PACKET_ID packetID);
	//void SendMsg(int iSessionID, const ::google::protobuf::Message& message);
	template<typename T>
	void SendMsg(int iSessionID, const T& message);
	//�������� OnReceive�ÿ� Queue�о��ֱ�.
	void PushQueue(int iSessionID, BYTE* pData, int iSize); 
	

//����
	//�� �����忡�� �̿��Ͽ� �ڵ� ��Ŷ�� ȣ��
	bool HandlePacket(int iSessionID, BYTE* pData, int iSize);


private://ó���� ����
	std::mutex m_mutex;
	std::queue<PacketUnit> m_PacketQueue;
	std::function<bool(int, BYTE*, int)> m_SendFunction;
private://���ο� ����
	//�޽��� ������ ����Ŀ
	std::unordered_map<PACKET_ID, std::function<std::shared_ptr<google::protobuf::Message>()>> m_MesaageFactory;
	//��Ŷ�� �Լ� �ڵ鷯
	std::unordered_map<PACKET_ID, PacketHandlerFunction> m_PacketHandler; // id / GameClient,byte*,int
private:

	bool Handle_CSP_Ping(int iSessionID, std::shared_ptr<google::protobuf::Message> message, int iSize);
	bool Handle_CSP_Echo(int iSessionID, std::shared_ptr<google::protobuf::Message> message, int iSize);
	bool Handle_CSP_Login(int iSessionID, std::shared_ptr<google::protobuf::Message> message, int iSize);
	bool Handle_CSP_CharList(int iSessionID, std::shared_ptr<google::protobuf::Message> message, int iSize);
	bool Handle_CSP_CreateChar(int iSessionID, std::shared_ptr<google::protobuf::Message> message, int iSize);
	bool Handle_CSP_SelectChar(int iSessionID, std::shared_ptr<google::protobuf::Message> message, int iSize);
};
