#include "stdafx.h"
#include "DBRequest_GetCharacterList.h"
#include "DBConnection.h"


DBRequest_GetCharacterList::DBRequest_GetCharacterList(
	int iAccountID,
	int iSeesion)
	: m_iAccountID(iAccountID),
	m_iSession(iSeesion)
{
}

void DBRequest_GetCharacterList::Execute(std::shared_ptr<DBConnection> db_connection)
{
	db_connection->StoredProcedure_GetCharacterList(this);
}