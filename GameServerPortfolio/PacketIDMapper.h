#pragma once
#include "jhnet.pb.h"

//벡터처럼 변수템플릿
template<typename T>
struct PacketIdMapper;


//등록하지않으면 ID값이 없어서 컴파일 에러가 나서 실수를 줄여줌

#pragma region CLIENT_TO_SERVER___CSP___C2S
template<>
struct PacketIdMapper<jhnet::CSP_Ping>
{
    static constexpr uint16_t ID = jhnet::PacketId::C2S_PING;
};

template<>
struct PacketIdMapper<jhnet::CSP_Echo>
{
    static constexpr uint16_t ID = jhnet::PacketId::C2S_ECHO;
};

template<>
struct PacketIdMapper<jhnet::CSP_Login>
{
    static constexpr uint16_t ID = jhnet::PacketId::C2S_LOGIN;
};

template<>
struct PacketIdMapper<jhnet::CSP_CharList>
{
    static constexpr uint16_t ID = jhnet::PacketId::C2S_CHAR_LIST;
};

template<>
struct PacketIdMapper<jhnet::CSP_CreateChar>
{
    static constexpr uint16_t ID = jhnet::PacketId::C2S_CREATE_CHAR;
};


template<>
struct PacketIdMapper<jhnet::CSP_SelectChar>
{
    static constexpr uint16_t ID = jhnet::PacketId::C2S_SELECT_CHAR;
};

template<>
struct PacketIdMapper<jhnet::CSP_MapLoadEnd>
{
    static constexpr uint16_t ID = jhnet::PacketId::C2S_MAP_LOAD_END;
};

template<>
struct PacketIdMapper<jhnet::CSP_Move>
{
    static constexpr uint16_t ID = jhnet::PacketId::C2S_MOVE;
};


#pragma endregion



#pragma region  SERVER_TO_CLIENT___SCP_S2C
template<>
struct PacketIdMapper<jhnet::SCP_Ping>
{
    static constexpr uint16_t ID = jhnet::PacketId::S2C_PING;
};

template<>
struct PacketIdMapper<jhnet::SCP_Echo>
{
    static constexpr uint16_t ID = jhnet::PacketId::S2C_ECHO;
};

template<>
struct PacketIdMapper<jhnet::SCP_Login>
{
    static constexpr uint16_t ID = jhnet::PacketId::S2C_LOGIN;
};

template<>
struct PacketIdMapper<jhnet::SCP_CharList>
{
    static constexpr uint16_t ID = jhnet::PacketId::S2C_CHAR_LIST;
};

template<>
struct PacketIdMapper<jhnet::SCP_CreateChar>
{
    static constexpr uint16_t ID = jhnet::PacketId::S2C_CREATE_CHAR;
};

template<>
struct PacketIdMapper<jhnet::SCP_SelectCharResult>
{
    static constexpr uint16_t ID = jhnet::PacketId::S2C_SELECT_CHAR_RESULT;
};

template<>
struct PacketIdMapper<jhnet::SCP_MyCharacterInfo>
{
    static constexpr uint16_t ID = jhnet::PacketId::S2C_MY_CHARACTER_INFO;
};

template<>
struct PacketIdMapper<jhnet::SCP_EnterGame>
{
    static constexpr uint16_t ID = jhnet::PacketId::S2C_ENTER_GAME;
};

template<>
struct PacketIdMapper<jhnet::SCP_Spawn>
{
    static constexpr uint16_t ID = jhnet::PacketId::S2C_SPAWN;
};

template<>
struct PacketIdMapper<jhnet::SCP_Despawn>
{
    static constexpr uint16_t ID = jhnet::PacketId::S2C_DESPAWN;
};

template<>
struct PacketIdMapper<jhnet::SCP_Move>
{
    static constexpr uint16_t ID = jhnet::PacketId::S2C_MOVE;
};

#pragma endregion







