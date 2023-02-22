#pragma once
#include "Creature.h"
#include "EquipmentBox.h"
#include "SkillBox.h"
#include "QuickSlotManager.h"
#include "InventoryManager.h"
#include "LockFreeQue.h"
#include "GameServerMessage.h"
#include "PartyManager.h"

struct st_TimerJob;

class CPlayer : public CCreature
{
public:			
	int64 _SessionId;
	int64 _AccountId;	

	CPartyManager _PartyManager;

	CEquipmentBox _Equipment;		
	
	CSkillBox _SkillBox;
	CQuickSlotManager _QuickSlotManager;

	st_Experience _Experience;				
	
	// 연속기 스킬
	CSkill* _ComboSkill;		

	vector<st_RayCatingPosition> _RayCastingPositions;
	
	CPlayer();	
	~CPlayer();		

	virtual void Update() override;

	virtual bool OnDamaged(CGameObject* Attacker, int32 Damage) override;

	virtual void Start() override;
	virtual void End() override;

	void RayCastingToFieldOfViewObjects();
protected:
	virtual bool UpdateSpawnIdle() override;
	virtual void UpdateIdle() override;
	virtual void UpdateMoving() override;	
	virtual void UpdateAttack() override;
	virtual void UpdateSpell() override;	
	virtual void UpdateGathering() override;
	virtual void UpdateReadyDead() override;
	virtual void UpdateDead() override;
private:
	void CheckFieldOfViewObject();	
};