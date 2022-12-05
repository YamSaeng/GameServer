#pragma once
#include "GameObject.h"

class CPartyManager
{
public:
	bool _IsPartyLeader;
	bool _IsParty;

	CPartyManager();
	~CPartyManager();
		
	vector<CPlayer*> GetPartyPlayerArray();

	void PartyManagerInit(CPlayer* InitPlayer);
	 
	bool PartyLeaderInvite(CPlayer* PartyPlayer);
	
	bool PartyInvite(CPlayer* PartyPlayer);
	void PartyInvited(CPlayer* InvitePlayer);

	bool PartyQuit();	
	void PartyQuited(int64 QuitPartyPlayerID);
	void PartyAllQuit();

	bool IsPartyMember(int64& PartyMemberID);
private:
	vector<CPlayer*> _PartyPlayers;	
	CPlayer* _OwnerPlayer;
};

