#pragma once
#include <memory>

//인터페이스
class DBRequest
{
public:
	//virtual DBRequest() = default;
	virtual ~DBRequest() = default;
	virtual void Execute(std::shared_ptr<class DBConnection> db_connection) = 0;
};

