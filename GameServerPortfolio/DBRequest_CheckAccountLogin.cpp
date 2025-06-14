#include "stdafx.h"
#include "DBRequest_CheckAccountLogin.h"
#include "DBConnection.h"

DBRequest_CheckAccountLogin::DBRequest_CheckAccountLogin(
	std::string id, 
	std::string pw, 
	int iSessionID)
	: m_id(std::move(id)),
	m_pw(std::move(pw)),
	m_iSessionID(iSessionID)
{
}

void DBRequest_CheckAccountLogin::Execute(std::shared_ptr<DBConnection> db_connection)
{
	db_connection->StoredProcedure_CheckAccountLogin(this);
}