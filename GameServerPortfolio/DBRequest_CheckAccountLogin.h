#pragma once
#include "DBRequest.h"

class DBRequest_CheckAccountLogin : public DBRequest
{
public:
    DBRequest_CheckAccountLogin(
        std::string id,
        std::string pw,
        std::shared_ptr<class GameClient> client);

    void Execute(std::shared_ptr<DBConnection> db_connection) override;

private:
    std::string _id;
    std::string _pw;
    std::weak_ptr<class GameClient> _client;
};