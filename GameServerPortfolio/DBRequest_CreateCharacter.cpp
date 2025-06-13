#include "stdafx.h"
#include "DBRequest_CreateCharacter.h"
#include "DBConnection.h"


DBRequest_CreateCharacter::DBRequest_CreateCharacter(
	std::wstring _nickname,
	int account_uid,
	int job_code,
	std::shared_ptr<class GameClient> client)
	: _nickname(std::move(_nickname)),
	_account_uid(account_uid),
	_job_code(std::move(job_code)),
	_client(client)
{
}



void DBRequest_CreateCharacter::Execute(std::shared_ptr<class DBConnection> db_connection)
{
	db_connection->StoredProcedure_CreateCharacter(this);
}
