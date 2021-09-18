	#pragma once
#include "GameObject.h"
#include "Inventory.h"
#include "SkillBox.h"
#include "QuickSlotManager.h"

class CPlayer : public CGameObject
{
public:	
	uint64 _AttackTick;
	en_AttackType _AttackType;

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

	void SetAttackMeleeType(en_AttackType AttackType, vector<CGameObject*> Targets);
	void SetAttackMagicType(en_AttackType AttackType, vector<CGameObject*> Targets);
protected:
	virtual void UpdateAttack();
	virtual void UpdateSpell();
};

