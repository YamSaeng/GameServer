#pragma once
#include "Creature.h"
#include "SkillBox.h"
#include "QuickSlotManager.h"
#include "InventoryManager.h"
#include "LockFreeQue.h"
#include "GameServerMessage.h"
#include "PartyManager.h"

class CPlayer : public CCreature
{
public:			
	int64 _SessionId;
	int64 _AccountId;	

	CPartyManager _PartyManager;		
	
	CSkillBox _SkillBox;
	CQuickSlotManager _QuickSlotManager;

	st_Experience _Experience;							
	
	CPlayer();	
	~CPlayer();		

	virtual void Update() override;

	virtual bool OnDamaged(CGameObject* Attacker, int32 Damage) override;

	virtual void Start() override;
	virtual void End() override;

protected:
	virtual bool UpdateSpawnIdle() override;
	virtual void UpdateIdle() override;
	virtual void UpdateMoving() override;	
	virtual void UpdateAttack() override;
	virtual void UpdateSpell() override;	
	virtual void UpdateGathering() override;	
	virtual void UpdateDead() override;
private:
	void CheckFieldOfViewObject();	
	void RayCastingToFieldOfViewObjects(vector<CGameObject*>* SpawnObjects, vector<CGameObject*>* DespawnObjects);
};