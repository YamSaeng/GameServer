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

	//---------------------
	// ���� �� ��ų ����
	//---------------------
	en_SkillType _SkillType;

	//--------------------
	// ���� ���� ��ų Job
	//--------------------
	st_TimerJob* _SkillJob;	

	int64 _SessionId;
	int64 _AccountId;

	CEquipment _Equipment;	
	CInventoryManager _InventoryManager;
	
	CSkillBox _SkillBox;
	CQuickSlotManager _QuickSlotManager;

	st_Experience _Experience;			

	// �þ� ���� ������Ʈ
	vector<st_FieldOfViewInfo> _FieldOfViewInfos;	

	CPlayer();	
	~CPlayer();		

	virtual void Update() override;

	virtual bool OnDamaged(CGameObject* Attacker, int32 Damage) override;
	virtual void OnDead(CGameObject* Killer) override;	

	void Init();	

	virtual void PositionReset() override;	
protected:
	virtual void UpdateMove();	
	virtual void UpdateAttack();
	virtual void UpdateSpell();	
};