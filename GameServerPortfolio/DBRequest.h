#pragma once
#include <memory>

//�������̽�
class DBRequest
{
public:
	//virtual DBRequest() = default;
	virtual ~DBRequest() = default;
	virtual void Execute(std::shared_ptr<class DBConnection> db_connection) = 0;
};

