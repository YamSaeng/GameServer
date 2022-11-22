#include "pch.h"
#include "PartyManager.h"

CPartyManager::CPartyManager()
{
    _IsPartyLeader = false;
    _IsParty = false;
}

CPartyManager::~CPartyManager()
{
}

vector<CPlayer*> CPartyManager::GetPartyPlayerArray()
{
    return _PartyPlayers;
}

void CPartyManager::PartyManagerInit(CPlayer* InitPlayer)
{
    _OwnerPlayer = InitPlayer;

    _PartyPlayers.push_back(InitPlayer);
}

bool CPartyManager::PartyLeaderInvite(CPlayer* PartyPlayer)
{
    if (_PartyPlayers.size() == 5)
    {
        return false;
    }

    _IsPartyLeader = true;
    _IsParty = true;

    _PartyPlayers.push_back(PartyPlayer);

    return true;
}

bool CPartyManager::PartyInvite(CPlayer* PartyPlayer)
{
    if (_PartyPlayers.size() == 5)
    {
        return false;
    }
    
    _IsParty = true;

    _PartyPlayers.push_back(PartyPlayer);

    return true;
}

void CPartyManager::PartyInvited(CPlayer* InvitePlayer)
{
    _PartyPlayers.push_back(InvitePlayer);
}

bool CPartyManager::PartyQuit(CPlayer* LeaderPartyPlayer)
{
    if (_PartyPlayers.size() < 2)
    {
        return false;
    }

    _IsParty = false;

    _PartyPlayers.clear();    

    _PartyPlayers.push_back(_OwnerPlayer);

    return true;
}
