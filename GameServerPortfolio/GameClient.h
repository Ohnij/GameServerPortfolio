#pragma once
#include "Client.h"



enum class ClientState
{
	CLIENT_CONNECTED,	//������ �α��� ��
	CLIENT_AUTHED,		//�α��� �����Ϸ� ����
	CLIENT_INGAME,		//���Ӿȿ� ����
};


class GameMap;
class GameClient : public Client
{
public:
	GameClient();
	virtual ~GameClient();

	//�½�����Ʈ
	inline void SetState(ClientState state) { _state = state; }
	inline ClientState GetState() const { return _state; }
	
	//�°��Ӹ�
	inline void SetGameMap(std::shared_ptr<GameMap> game_map) { _game_map = game_map; }
	inline std::shared_ptr<GameMap> GetGameMap() const { return _game_map; }

	//���÷��̾�
	//inline void SetState(ClientState state) { _state = state; }
	//inline ClientState GetState() const { return _state; }


private:
	ClientState _state = ClientState::CLIENT_CONNECTED;
	std::shared_ptr<GameMap> _game_map;	
	//�÷��̾�����?
};

