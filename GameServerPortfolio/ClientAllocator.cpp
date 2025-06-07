#include "stdafx.h"
#include "ClientAllocator.h"
#include "ObjectPool.h"
#include "Client.h"

std::shared_ptr<Client> ClientAllocator::Create() const
{
	return _creater();
}

void ClientAllocator::Release(std::shared_ptr<Client> client) const
{
	return _releaser(client);
}

ClientAllocator::ClientAllocator()
{
	//std::function<std::shared_ptr<class Client>(SOCKET, int)> create;
	_creater = []() { return PoolManager::GetPool<Client>().Get(); };

	//std::function<void(std::shared_ptr<class Client>)> release;
	_releaser = [](std::shared_ptr<class Client> client) {
		PoolManager::GetPool<Client>().Return(std::move(client));
	};
}
