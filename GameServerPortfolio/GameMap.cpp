#include "stdafx.h"
#include "GameMap.h"

GameMap::GameMap(int map_id)
	: _map_id(map_id)
{
	
}

void GameMap::Init()
{
	//맵별로 뭔가 로드하는 수단?
}

void GameMap::StartThread()
{
	//_logic_thread = std::thread([game_map = shared_from_this()] 
	//{
	//	game_map->Loop();
	//});
}

void GameMap::Loop()
{
	//틱시간 정하기

	while (true)
	{
		
		//queue 빼기
		while (true)
		{
			//lock
			//if(queue empty)
			//	break;
			//queue pop
			//
			//handle packet
			//
			//시간체크
			//queue시간이 일정시간넘어가면 
			//	break;
		}

		//게임로직
		//몬스터

		
		//틱시간 쉬기(주기정렬) (이걸안하면 특정맵은 몬스터가 빠르게 움직이거나 함)
	}
}

