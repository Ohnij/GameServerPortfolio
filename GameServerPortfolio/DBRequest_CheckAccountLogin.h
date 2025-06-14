#pragma once
#include "DBRequest.h"

class DBRequest_CheckAccountLogin : public DBRequest
{
public:
    DBRequest_CheckAccountLogin(
        std::string id,
        std::string pw,
        int iSessionID);

    void Execute(std::shared_ptr<class DBConnection> db_connection) override;
    void Callback() override
    {

    }
private:
    friend class DBConnection;

    std::string m_id;
    std::string m_pw;
    int m_iSessionID;
};