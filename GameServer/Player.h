	#pragma once
#include "GameObject.h"
#include "Inventory.h"
#include "SkillBox.h"
#include "QuickSlotManager.h"

struct st_TimerJob;

class CPlayer : public CGameObject
{
public:	
	uint64 _AttackTick;
	uint64 _SpellTick;
	en_SkillType _SkillType;
	st_TimerJob* _SkillJob;

	int64 _SessionId;
	int64 _AccountId;

	CInventory _Inventory;
	CSkillBox _SkillBox;
	CQuickSlotManager _QuickSlotManager;

	CPlayer();	
	~CPlayer();		

	virtual void Update() override;

	virtual void OnDamaged(CGameObject* Attacker, int32 Damage) override;
	virtual void OnDead(CGameObject* Killer) override;	

protected:
	virtual void UpdateAttack();
	virtual void UpdateSpell();
};

