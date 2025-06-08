#pragma once

typedef uint16_t PACKET_ID, PACKET_LEN;

#pragma pack(push, 1)
struct PacketHeader
{
	PACKET_LEN packet_length;
	PACKET_ID packet_id;
};
#pragma pack(pop)
