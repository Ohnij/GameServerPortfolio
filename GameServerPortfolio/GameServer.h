#pragma once
#include "IocpServer.h"



class GameServer: public IocpServer
{
public:
    bool Start(USHORT port);
      
protected:
    virtual void OnConnect(int iSessionID) override;
    virtual void OnReceive(int iSessionID, BYTE* pData, int iSize) override;
    virtual void OnSend(int iSessionID, int iSize) override;
    virtual void OnDisconnect(int iSessionID) override;
};

