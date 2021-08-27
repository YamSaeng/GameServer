	#pragma once
#include "GameObject.h"
#include "Inventory.h"

class CPlayer : public CGameObject
{
public:	
	int64 _SessionId;
	int64 _AccountId;

	CInventory _Inventory;

	CPlayer();
	CPlayer(st_GameObjectInfo _PlayerInfo);
	~CPlayer();		

	virtual void OnDamaged(CGameObject* Attacker, int32 Damage) override;
	virtual void OnDead(CGameObject* Killer) override;
};

