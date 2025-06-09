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

private:
	bool Release();
	void PrintErrorMessage(SQLSMALLINT handleType, SQLHANDLE handle);
	SQLHENV _h_env = nullptr;
	SQLHDBC _h_dbc = nullptr;

public: //DB Procedure function

	void StoredProcedure_CheckAccountLogin(class DBRequest_CheckAccountLogin* request);
	void StoredProcedure_GetCharacterList(class DBRequest_GetCharacterList* request);
	void StoredProcedure_CreateCharacter(class DBRequest_CreateCharacter* request);
	

};

