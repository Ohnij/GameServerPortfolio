#pragma once
#include <sqlext.h>

class DBConnection
{
public:
	DBConnection();
	~DBConnection();


	bool Init();
	bool TestQuery();
	bool TestQuery2();

private:
	bool Release();
	void PrintErrorMessage(SQLSMALLINT handleType, SQLHANDLE handle);
	SQLHENV _h_env = nullptr;
	SQLHDBC _h_dbc = nullptr;
	SQLHSTMT _h_stmt = nullptr;
};

