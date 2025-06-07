#pragma once
#include <sql.h>
#include <sqlext.h>

class DBConnection
{
public:
	DBConnection();
	~DBConnection();


	bool Init();
	bool TestQuery();
	bool TestQuery2();
	bool TestQuery3(const std::string& id, const std::string& pw, int& out_uid, int& out_result_code);

private:
	bool Release();
	void PrintErrorMessage(SQLSMALLINT handleType, SQLHANDLE handle);
	SQLHENV _h_env = nullptr;
	SQLHDBC _h_dbc = nullptr;

public: //DB Procedure function

	
	void CallCheckAccountLogin(const std::string& id, const std::string& pw, int& out_uid, int& out_result_code);


};

