	#pragma once
#include "GameObject.h"
#include "Equipment.h"
#include "Inventory.h"
#include "SkillBox.h"
#include "QuickSlotManager.h"

struct st_TimerJob;

class CPlayer : public CGameObject
{
public:	
	uint64 _AttackTick;
	uint64 _SpellTick;

	//---------------------
	// 시전 한 스킬 종류
	//---------------------
	en_SkillType _SkillType;

	//--------------------
	// 시전 중인 스킬 Job
	//--------------------
	st_TimerJob* _SkillJob;	

	int64 _SessionId;
	int64 _AccountId;

	CEquipment _Equipment;
	CInventory _Inventory;
	CSkillBox _SkillBox;
	CQuickSlotManager _QuickSlotManager;

	st_Experience _Experience;

	CPlayer();	
	~CPlayer();		

	virtual void Update() override;

	virtual bool OnDamaged(CGameObject* Attacker, int32 Damage) override;
	virtual void OnDead(CGameObject* Killer) override;	

protected:
	virtual void UpdateAttack();
	virtual void UpdateSpell();
};

