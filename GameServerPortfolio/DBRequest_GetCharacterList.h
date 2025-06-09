#pragma once
#include "DBRequest.h"

class DBRequest_GetCharacterList : public DBRequest
{
public:
    DBRequest_GetCharacterList(
		int account_uid,
		std::shared_ptr<class GameClient> client);

	void Execute(std::shared_ptr<class DBConnection> db_connection) override;
private:
	friend class DBConnection;

	int _account_uid;
	std::weak_ptr<class GameClient> _client;

};