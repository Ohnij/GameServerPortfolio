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
	//std::queue<GameMessage> _message_queue; //<<여기에 패킷을 파싱해서 넣기
	//GameMessage > 
	// user / packetid / packet

	
	//맵에 있는 몬스터?


	//enter user
	//leave user
	// 
	//User들을 관리하는 방법 UserManager등으로 뭔가 해야할것
	//std::unordered_map < int64_t, std::shared_ptr<User>> _users;
};


//user  < - > client (weak_ptr?)
//
