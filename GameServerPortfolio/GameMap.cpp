#include "stdafx.h"
#include "GameMap.h"

GameMap::GameMap(int map_id)
	: _map_id(map_id)
{
	
}

void GameMap::Init()
{
	//�ʺ��� ���� �ε��ϴ� ����?
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
	//ƽ�ð� ���ϱ�

	while (true)
	{
		
		//queue ����
		while (true)
		{
			//lock
			//if(queue empty)
			//	break;
			//queue pop
			//
			//handle packet
			//
			//�ð�üũ
			//queue�ð��� �����ð��Ѿ�� 
			//	break;
		}

		//���ӷ���
		//����

		
		//ƽ�ð� ����(�ֱ�����) (�̰ɾ��ϸ� Ư������ ���Ͱ� ������ �����̰ų� ��)
	}
}

