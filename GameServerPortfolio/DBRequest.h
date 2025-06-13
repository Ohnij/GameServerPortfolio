#pragma once
#include <memory>

//�������̽�
class DBRequest
{
public:
	//virtual DBRequest() = default;
	virtual ~DBRequest() = default;


	//�����Լ� ��ü�� ��ü��
	virtual void Execute(std::shared_ptr<class DBConnection> db_connection) = 0;
	virtual void Callback() = 0;
};

