#pragma once
#include "Client.h"



enum class ClientState
{
	CLIENT_CONNECTED,	//연결후 로그인 전
	CLIENT_AUTHED,		//로그인 인증완료 상태
	CLIENT_INGAME,		//게임안에 진입
};


class GameMap;
class GameClient : public Client
{
public:
	GameClient();
	virtual ~GameClient();

	//셋스테이트
	inline void SetState(ClientState state) { _state = state; }
	inline ClientState GetState() const { return _state; }
	
	//셋게임맵
	inline void SetGameMap(std::shared_ptr<GameMap> game_map) { _game_map = game_map; }
	inline std::shared_ptr<GameMap> GetGameMap() const { return _game_map; }

	//셋플레이어
	//inline void SetState(ClientState state) { _state = state; }
	//inline ClientState GetState() const { return _state; }


private:
	ClientState _state = ClientState::CLIENT_CONNECTED;
	std::shared_ptr<GameMap> _game_map;	
	//플레이어정보?
};

