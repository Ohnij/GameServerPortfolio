#pragma once
#include <memory>

//인터페이스
class DBRequest
{
public:
	//virtual DBRequest() = default;
	virtual ~DBRequest() = default;


	//실행함수 객체와 일체형
	virtual void Execute(std::shared_ptr<class DBConnection> db_connection) = 0;
	virtual void Callback() = 0;
};

