#pragma once
#include <thread>
#include <mutex>


class GameMap : public std::enable_shared_from_this<GameMap>
{
public:
	GameMap(int map_id);

	void Init();
	void StartThread();
	//void PushMessage(GameMessage game_message);
	

private:
	void Loop();
private:
	int _map_id;
	std::thread _logic_thread;

	std::mutex _mutex;
	//std::queue<GameMessage> _message_queue; //<<���⿡ ��Ŷ�� �Ľ��ؼ� �ֱ�
	//GameMessage > 
	// user / packetid / packet

	
	//�ʿ� �ִ� ����?


	//enter user
	//leave user
	// 
	//User���� �����ϴ� ��� UserManager������ ���� �ؾ��Ұ�
	//std::unordered_map < int64_t, std::shared_ptr<User>> _users;
};


//user  < - > client (weak_ptr?)
//
