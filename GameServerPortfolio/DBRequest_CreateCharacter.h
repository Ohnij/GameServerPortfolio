#pragma once
#include "DBRequest.h"


class DBRequest_CreateCharacter : public DBRequest
{
public:
    DBRequest_CreateCharacter(
        std::wstring wsNickname,
        int iAccountID,
        int iJobCode,
        int iSessionID);

    void Execute(std::shared_ptr<class DBConnection> db_connection) override;
    void Callback() override
    {

    }
private:
    friend class DBConnection;

    std::wstring m_wsNickname;
    int m_iAccountID;
    int m_iJobCode;
    int m_iSessionID;
};