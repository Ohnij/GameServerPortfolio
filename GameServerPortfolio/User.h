#pragma once



enum class UserState
{
	USER_NONE,		//사용중이지 않음
	USER_AUTHED,		//로그인 인증완료 상태
	USER_INGAME,		//게임안에 진입
};

class User : public std::enable_shared_from_this<User>
{
public:
	User();
	virtual ~User();

	//셋스테이트
	inline void SetState(UserState state) { _state = state; }
	inline UserState GetState() const { return _state; }
	
	//셋게임맵
	inline void SetGameMap(std::shared_ptr<class GameMap> game_map) { _game_map = game_map; }
	inline std::shared_ptr<class GameMap> GetGameMap() const { return _game_map; }

	//셋플레이어
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
	//플레이어정보?

	int _account_uid = -1;
	int _client_number;			// 클라이언트와 연결된 번호
};

