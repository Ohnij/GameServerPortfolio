#include "stdafx.h"
#include "DBRequest_CreateCharacter.h"
#include "DBConnection.h"


DBRequest_CreateCharacter::DBRequest_CreateCharacter(
	std::wstring wsNickname,
	int iAccountID,
	int iJobCode,
	int iSessionID)
	: m_wsNickname(std::move(wsNickname)),
	m_iAccountID(iAccountID),
	m_iJobCode(iJobCode),
	m_iSessionID(iSessionID)
{
}



void DBRequest_CreateCharacter::Execute(std::shared_ptr<class DBConnection> db_connection)
{
	db_connection->StoredProcedure_CreateCharacter(this);
}
