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
			//GameClient�� �ƴϹǷ� Ǯ�� �ݳ����� �ʰ� �Ҹ�ǵ��� �Ѵ�.
			//TODO : �α׸Ŵ����� DEBUG�ÿ��� �α׳����
			return;
		}
		PoolManager::GetPool<GameClient>().Return(gc);
	};
}
