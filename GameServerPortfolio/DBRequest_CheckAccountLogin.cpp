#include "stdafx.h"
#include "DBRequest_CheckAccountLogin.h"
#include "DBConnection.h"
#include "GameClient.h"
#include "PacketManager.h"

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

	db_connection->CallCheckAccountLogin(_id, _pw, account_uid, result_code);
	
	auto client = _client.lock();
	if (!client)
	{
		std::cerr << "DBRequest_CheckAccountLogin :: client null \n";
		return;
	}
	jhnet::SCP_Login res;
	res.set_account_uid(account_uid);
	res.set_login_ok(false);
	if (result_code == 0)
	{
		res.set_login_ok(true);
		client->Login(account_uid);
		std::cerr << "로그인성공 \n";
	}
	else
	{
		std::cerr << "로그인실패 \n";
	}
	PacketManager::Instance().Send(client, res, jhnet::PacketId::S2C_LOGIN);
}