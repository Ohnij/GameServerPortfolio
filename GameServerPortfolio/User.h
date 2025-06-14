#pragma once



enum class UserState
{
	USER_NONE,			//사용중이지 않음
	USER_CONNECTED,		//로그인 전 상태
	USER_AUTHED,		//로그인 인증완료 상태
	USER_INGAME,		//게임안에 진입
};

class User : public std::enable_shared_from_this<User>
{
public:
	User();
	virtual ~User();

	//셋스테이트
	inline void SetState(UserState state) { m_State = state; }
	inline UserState GetState() const { return m_State; }
	
	//셋게임맵
	inline void SetGameMap(std::shared_ptr<class GameMap> pGameMap) { m_pGameMap = pGameMap; }
	inline std::shared_ptr<class GameMap> GetGameMap() const { return m_pGameMap; }

	//셋플레이어
	//inline void SetUnit(unit unit) { unit = unit; }
	//inline Unit GetUnit() const { return unit; }

	inline int GetAccountID() { return m_iAccountID; }
	inline int GetSessionID() { return m_iSessionID; }


	//void OnReceive(BYTE* data, int size);
	void Connect(int iSessionID);
	void Login(int iAccountID)
	{
		SetState(UserState::USER_AUTHED);
		m_iAccountID = iAccountID;
	}

	inline std::shared_ptr<User> GetSelf() { return std::static_pointer_cast<User>(shared_from_this()); }
private:
	UserState m_State = UserState::USER_NONE;
	std::shared_ptr<class GameMap> m_pGameMap;	
	//플레이어정보?

	int m_iSessionID;
	int m_iAccountID = -1;
};

