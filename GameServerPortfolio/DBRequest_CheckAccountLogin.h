#pragma once
#include "DBRequest.h"

class DBRequest_CheckAccountLogin : public DBRequest
{
public:
    DBRequest_CheckAccountLogin(
        std::string id,
        std::string pw,
        int client_id);

    void Execute(std::shared_ptr<class DBConnection> db_connection) override;

private:
    friend class DBConnection;

    std::string _id;
    std::string _pw;
    int _client_id;
};