#pragma once
#include "ClientAllocator.h"
class GameClientAllocater :public ClientAllocator
{
public:
	GameClientAllocater();
	std::shared_ptr<class Client> Create() const override;
	void Release(std::shared_ptr<class Client> client) const override;
};

