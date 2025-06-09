#include "stdafx.h"
#include "DBRequest_GetCharacterList.h"
#include "DBConnection.h"
#include "GameClient.h"

DBRequest_GetCharacterList::DBRequest_GetCharacterList(
	int account_uid, 
	std::shared_ptr<class GameClient> client)
	: _account_uid(account_uid),
	_client(client)

{
}


void DBRequest_GetCharacterList::Execute(std::shared_ptr<DBConnection> db_connection)
{
	db_connection->StoredProcedure_GetCharacterList(this);
}