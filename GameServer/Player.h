	#pragma once
#include "GameObject.h"
#include "Inventory.h"
#include "SkillBox.h"
#include "QuickSlotManager.h"

class CPlayer : public CGameObject
{
public:	
	uint64 _AttackTick;
	en_SkillType _SkillType;

	int64 _SessionId;
	int64 _AccountId;

	CInventory _Inventory;
	CSkillBox _SkillBox;
	CQuickSlotManager _QuickSlotManager;

	CPlayer();
	CPlayer(st_GameObjectInfo _PlayerInfo);
	~CPlayer();		

	virtual void Update() override;

	virtual void OnDamaged(CGameObject* Attacker, int32 Damage) override;
	virtual void OnDead(CGameObject* Killer) override;	

protected:
	virtual void UpdateAttack();
	virtual void UpdateSpell();
};

