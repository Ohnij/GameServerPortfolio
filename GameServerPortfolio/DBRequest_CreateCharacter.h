#pragma once
#include "DBRequest.h"


class DBRequest_CreateCharacter : public DBRequest
{
public:
    DBRequest_CreateCharacter(
        std::wstring _nickname,
        int account_uid,
        int job_code,
        std::shared_ptr<class GameClient> client);

    void Execute(std::shared_ptr<class DBConnection> db_connection) override;

private:
    friend class DBConnection;

    std::wstring _nickname;
    int _account_uid;
    int _job_code;
    std::weak_ptr<class GameClient> _client;
};