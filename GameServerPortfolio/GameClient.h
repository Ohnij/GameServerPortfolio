#pragma once
#include "Client.h"



enum class ClientState
{
	CLIENT_CONNECTED,	//연결후 로그인 전
	CLIENT_AUTHED,		//로그인 인증완료 상태
	CLIENT_INGAME,		//게임안에 진입
};

class GameClient : public Client 
{
public:
	GameClient();
	virtual ~GameClient();

	//셋스테이트
	inline void SetState(ClientState state) { _state = state; }
	inline ClientState GetState() const { return _state; }
	
	//셋게임맵
	inline void SetGameMap(std::shared_ptr<class GameMap> game_map) { _game_map = game_map; }
	inline std::shared_ptr<class GameMap> GetGameMap() const { return _game_map; }

	//셋플레이어
	//inline void SetState(ClientState state) { _state = state; }
	//inline ClientState GetState() const { return _state; }

	void OnReceive(BYTE* data, int size) override;

	void Login(int account_uid)
	{
		SetState(ClientState::CLIENT_AUTHED);
		_account_uid = account_uid;
	}


	//Shared_ptr<GameClinet> 받기  //이중 Enable_shared_from_this 불가능 -> _Throw_bad_weak_ptr 에러 발생 
	//Shared_ptr<GameClinet>를 매개변수로 하는곳은 항상 이 값으로할것. 
	inline std::shared_ptr<GameClient> GetSelf() { return std::static_pointer_cast<GameClient>(shared_from_this()); }
private:
	ClientState _state = ClientState::CLIENT_CONNECTED;
	std::shared_ptr<class GameMap> _game_map;	
	//플레이어정보?

	int _account_uid = -1;
};

