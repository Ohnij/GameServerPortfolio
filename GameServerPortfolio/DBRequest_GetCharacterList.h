#pragma once
#include "DBRequest.h"

class DBRequest_GetCharacterList : public DBRequest
{
public:
    DBRequest_GetCharacterList(
		int iAccountID,
		int iSeesion);

	void Execute(std::shared_ptr<class DBConnection> db_connection) override;
	void Callback() override
	{

	}
private:
	friend class DBConnection;

	int m_iAccountID;
	int m_iSession;
};