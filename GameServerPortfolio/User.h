#pragma once



enum class UserState
{
	USER_NONE,		//��������� ����
	USER_AUTHED,		//�α��� �����Ϸ� ����
	USER_INGAME,		//���Ӿȿ� ����
};

class User : public std::enable_shared_from_this<User>
{
public:
	User();
	virtual ~User();

	//�½�����Ʈ
	inline void SetState(UserState state) { _state = state; }
	inline UserState GetState() const { return _state; }
	
	//�°��Ӹ�
	inline void SetGameMap(std::shared_ptr<class GameMap> game_map) { _game_map = game_map; }
	inline std::shared_ptr<class GameMap> GetGameMap() const { return _game_map; }

	//���÷��̾�
	//inline void SetState(ClientState state) { _state = state; }
	//inline ClientState GetState() const { return _state; }

	inline int GetAccountUid() { return _account_uid; }


	//void OnReceive(BYTE* data, int size);

	void Login(int account_uid)
	{
		SetState(UserState::USER_AUTHED);
		_account_uid = account_uid;
	}

	inline std::shared_ptr<User> GetSelf() { return std::static_pointer_cast<User>(shared_from_this()); }
private:
	UserState _state = UserState::USER_AUTHED;
	std::shared_ptr<class GameMap> _game_map;	
	//�÷��̾�����?

	int _account_uid = -1;
	int _client_number;			// Ŭ���̾�Ʈ�� ����� ��ȣ
};

