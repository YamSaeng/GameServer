#pragma once
#include "GameObject.h"
#include "Equipment.h"
#include "SkillBox.h"
#include "QuickSlotManager.h"
#include "InventoryManager.h"
#include "LockFreeQue.h"
#include "GameServerMessage.h"

struct st_TimerJob;

class CPlayer : public CGameObject
{
public:			
	uint64 _DefaultAttackTick;	
	uint64 _SpellTick;		

	int64 _SessionId;
	int64 _AccountId;

	CEquipment _Equipment;	
	CInventoryManager _InventoryManager;
	
	CSkillBox _SkillBox;
	CQuickSlotManager _QuickSlotManager;

	st_Experience _Experience;			

	// �þ� ���� ������Ʈ
	vector<st_FieldOfViewInfo> _FieldOfViewInfos;	

	// ���� ��� ���� ��ų
	CSkill* _CurrentSkill;
	// ���ӱ� ��ų
	CSkill* _ComboSkill;		

	CPlayer();	
	~CPlayer();		

	virtual void Update() override;

	virtual bool OnDamaged(CGameObject* Attacker, int32 Damage) override;

	virtual void Init() override;	

	virtual void PositionReset() override;	
protected:
	virtual void UpdateMove();	
	virtual void UpdateAttack();
	virtual void UpdateSpell();	
};