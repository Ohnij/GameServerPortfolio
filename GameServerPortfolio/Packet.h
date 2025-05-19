#pragma once
#include "jhnet.pb.h"

//enum class PACKET_TYPE : uint16_t
//{
//	CS_PING =1001,
//	CS_ECHO,
//	SC_PING =2001,
//	SC_ECHO,
//};


typedef uint16_t PACKET_ID, PACKET_LEN;

#pragma pack(push, 1)
struct PacketHeader
{
	PACKET_LEN packet_length;
	PACKET_ID packet_id;
};
#pragma pack(pop)


//#pragma pack(push, 1)
//struct CSP_ECHO : public PacketHeader
//{
//	int number;
//	char message[128];
//};
//#pragma pack(pop)
//
//#pragma pack(push, 1)
//struct SCP_ECHO : public PacketHeader
//{
//	SCP_ECHO() { packet_length = sizeof(SCP_ECHO); packet_id = static_cast<uint16_t>(PACKET_TYPE::SC_ECHO); }
//	int number;
//	char message[128];
//};
//#pragma pack(pop)
