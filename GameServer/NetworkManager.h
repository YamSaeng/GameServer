#pragma once
#include"GameServer.h"

class CNetworkManager
{
public:	

	void SetGameServer(CGameServer* GameServer);
	CGameServer* GetGameServer();
private:
	CGameServer* _GameServer;
};

