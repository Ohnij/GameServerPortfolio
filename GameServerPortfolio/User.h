#pragma once



enum class UserState
{
	USER_NONE,			//��������� ����
	USER_CONNECTED,		//�α��� �� ����
	USER_AUTHED,		//�α��� �����Ϸ� ����
	USER_INGAME,		//���Ӿȿ� ����
};

class User : public std::enable_shared_from_this<User>
{
public:
	User();
	virtual ~User();

	//�½�����Ʈ
	inline void SetState(UserState state) { m_State = state; }
	inline UserState GetState() const { return m_State; }
	
	//�°��Ӹ�
	inline void SetGameMap(std::shared_ptr<class GameMap> pGameMap) { m_pGameMap = pGameMap; }
	inline std::shared_ptr<class GameMap> GetGameMap() const { return m_pGameMap; }

	//���÷��̾�
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
	//�÷��̾�����?

	int m_iSessionID;
	int m_iAccountID = -1;
};

