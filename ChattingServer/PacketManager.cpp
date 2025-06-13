#include "stdfx.h"
#include "PacketManager.h"


PacketManager::PacketManager()
{

}


PacketManager::~PacketManager()
{

}


PacketManager& PacketManager::Instance()
{
	static PacketManager instance;
	return instance;
}

void PacketManager::Init(std::function<bool(int, BYTE*, int)> send_func)
{
	m_Running = true;
	m_SendFunction = send_func;
	m_WorkerThread = std::thread([this]() { Run(); });
}

void PacketManager::PushPacket(int iSessionID, BYTE* pData, int iSIze)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_PacketQueue.push({iSessionID, std::vector<BYTE>(pData, pData + iSIze)});
	m_cv.notify_one();
}

void PacketManager::Run()
{
	while (m_Running)
	{
		Packet msg;

		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_cv.wait(lock, [&]() { return !m_PacketQueue.empty(); });

			msg = std::move(m_PacketQueue.front());
			m_PacketQueue.pop();
		}
		//����ϱ�
		ParsePacket(msg.iSessionID, msg.data.data(),msg.data.size());
	}
}




void PacketManager::Send(int iSessionID, const::google::protobuf::Message& packet, PACKET_ID PacketID)
{
	//user , msg, packet_id
	int data_size = packet.ByteSizeLong();
	int packet_size = sizeof(PacketHeader) + data_size;

	//����Ʈ�迭 ����
	std::vector<BYTE> buffer(packet_size);

	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer.data());
	header->m_Length = packet_size;
	header->m_ID = PacketID;

	packet.SerializeToArray(buffer.data() + sizeof(PacketHeader), packet_size);


	m_SendFunction(iSessionID, buffer.data(), packet_size);
}

void PacketManager::ParsePacket(int iSessionID, BYTE* pData, int iSize)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(pData);
	BYTE* payload = reinterpret_cast<BYTE*>(pData + sizeof(PacketHeader));

	std::shared_ptr<jhnet::CSP_Echo> packet = std::make_shared<jhnet::CSP_Echo>();
	//���⼭ �����Ͱ� ����� (data�� �������� ������� ���簡 �Ȼ���!!!)
	if (!packet->ParseFromArray(payload, header->m_Length - sizeof(PacketHeader)))
	{
		//��Ŷ �Ľ� ����
		std::cerr << "��Ŷ ����ȭ ����.." << header->m_ID << "\n";
		return;
	}

	std::cout << "Session[" << iSessionID << "] echo: number=" << packet->number() << ", message=" << packet->message() << "\n";

	jhnet::SCP_Echo res;
	res.set_number(packet->number());
	res.set_message(packet->message());

	Send(iSessionID, res, jhnet::PacketId::S2C_ECHO);
	return;
}
