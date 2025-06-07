#pragma once
#include "IOCPDefine.h"
#include <functional>


using ClientCreater = std::function<std::shared_ptr<class Client>()>;
using ClientDeleter = std::function<void(std::shared_ptr<class Client>)>;
class ClientAllocator
{
public:
	ClientAllocator();

	virtual std::shared_ptr<class Client> Create() const;
	virtual void Release(std::shared_ptr<class Client> client) const;
protected:
	ClientCreater _creater;
	ClientDeleter _releaser;
};

