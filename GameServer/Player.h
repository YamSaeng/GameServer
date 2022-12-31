#pragma once
#include "GameObject.h"
#include "Equipment.h"
#include "SkillBox.h"
#include "QuickSlotManager.h"
#include "InventoryManager.h"
#include "LockFreeQue.h"
#include "GameServerMessage.h"
#include "PartyManager.h"

struct st_TimerJob;

class CPlayer : public CGameObject
{
public:			
	int64 _SessionId;
	int64 _AccountId;	

	CPartyManager _PartyManager;

	CEquipment _Equipment;	
	CInventoryManager _InventoryManager;
	
	CSkillBox _SkillBox;
	CQuickSlotManager _QuickSlotManager;

	st_Experience _Experience;			

	// �þ� ���� ������Ʈ
	vector<st_FieldOfViewInfo> _FieldOfViewInfos;	
	
	// ���ӱ� ��ų
	CSkill* _ComboSkill;		

	// �⺻ ���� Ȱ��ȭ ����
	bool _OnPlayerDefaultAttack;		

	CPlayer();	
	~CPlayer();		

	virtual void Update() override;

	virtual bool OnDamaged(CGameObject* Attacker, int32 Damage) override;

	virtual void Start() override;
	virtual void End() override;

	virtual void PositionReset() override;	
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