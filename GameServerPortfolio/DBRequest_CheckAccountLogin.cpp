#include "stdafx.h"
#include "DBRequest_CheckAccountLogin.h"
#include "DBConnection.h"
#include "GameClient.h"

DBRequest_CheckAccountLogin::DBRequest_CheckAccountLogin(
	std::string id, 
	std::string pw, 
	std::shared_ptr<class GameClient> client)
	: _id(std::move(id)),
	_pw(std::move(pw)),
	_client(client)
{
}

void DBRequest_CheckAccountLogin::Execute(std::shared_ptr<DBConnection> db_connection)
{
    int account_uid = -1;
    int result_code = 2;


}