#include "stdafx.h"
#include "GameClientAllocater.h"
#include "ObjectPool.h"
#include "GameClient.h"
std::shared_ptr<Client> GameClientAllocater::Create() const
{
	return _creater();
}

void GameClientAllocater::Release(std::shared_ptr<Client> client) const
{
	return _releaser(client);
}

GameClientAllocater::GameClientAllocater()
{
	//std::function<std::shared_ptr<class Client>(SOCKET, int)> create;
	_creater = []() {

		auto gc = PoolManager::GetPool<GameClient>().Get();
		return std::static_pointer_cast<Client>(gc);
	};

	//std::function<void(std::shared_ptr<class Client>)> release;
	_releaser = [](std::shared_ptr<Client> client) {
		auto gc = std::dynamic_pointer_cast<GameClient>(client);
		if (!gc) 
		{
			//GameClient가 아니므로 풀에 반납하지 않고 소멸되도록 한다.
			//TODO : 로그매니저로 DEBUG시에만 로그남기기
			return;
		}
		PoolManager::GetPool<GameClient>().Return(gc);
	};
}
