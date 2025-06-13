#pragma once
#include <functional>
#include "IOCPDefine.h"
#include "jhnet.pb.h"

#include <mutex>

class PacketManager
{
private:
    PacketManager();
    ~PacketManager();
    struct Packet
    {
        int iSessionID;
        std::vector<BYTE> data;
    };
public:
	static PacketManager& Instance();

    void Init(std::function<bool(int, BYTE*, int)> send_func);

    void PushPacket(int iSessionID, BYTE* pData, int iSIze);
    void Run();


    void Send(int iSessionID, const::google::protobuf::Message& packet, PACKET_ID PacketID);
    void ParsePacket(int iSessionID, BYTE* pData, int iSize);
private:
    std::mutex m_mutex;

    std::queue<Packet> m_PacketQueue;
    std::condition_variable m_cv;
    std::thread m_WorkerThread;
    std::function<bool(int, BYTE*, int)> m_SendFunction;
    bool m_Running = false;
};
