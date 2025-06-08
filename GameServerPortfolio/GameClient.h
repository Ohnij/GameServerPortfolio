#pragma once
#include "Client.h"



enum class ClientState
{
	CLIENT_CONNECTED,	//������ �α��� ��
	CLIENT_AUTHED,		//�α��� �����Ϸ� ����
	CLIENT_INGAME,		//���Ӿȿ� ����
};

class GameClient : public Client 
{
public:
	GameClient();
	virtual ~GameClient();

	//�½�����Ʈ
	inline void SetState(ClientState state) { _state = state; }
	inline ClientState GetState() const { return _state; }
	
	//�°��Ӹ�
	inline void SetGameMap(std::shared_ptr<class GameMap> game_map) { _game_map = game_map; }
	inline std::shared_ptr<class GameMap> GetGameMap() const { return _game_map; }

	//���÷��̾�
	//inline void SetState(ClientState state) { _state = state; }
	//inline ClientState GetState() const { return _state; }

	void OnReceive(BYTE* data, int size) override;

	void Login(int account_uid)
	{
		SetState(ClientState::CLIENT_AUTHED);
		_account_uid = account_uid;
	}


	//Shared_ptr<GameClinet> �ޱ�  //���� Enable_shared_from_this �Ұ��� -> _Throw_bad_weak_ptr ���� �߻� 
	//Shared_ptr<GameClinet>�� �Ű������� �ϴ°��� �׻� �� �������Ұ�. 
	inline std::shared_ptr<GameClient> GetSelf() { return std::static_pointer_cast<GameClient>(shared_from_this()); }
private:
	ClientState _state = ClientState::CLIENT_CONNECTED;
	std::shared_ptr<class GameMap> _game_map;	
	//�÷��̾�����?

	int _account_uid = -1;
};

