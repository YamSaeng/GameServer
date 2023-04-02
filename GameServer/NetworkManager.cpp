#include "pch.h"
#include "NetworkManager.h"

void CNetworkManager::SetGameServer(CGameServer* GameServer)
{
    _GameServer = GameServer;
}

CGameServer* CNetworkManager::GetGameServer()
{
    return _GameServer;
}
