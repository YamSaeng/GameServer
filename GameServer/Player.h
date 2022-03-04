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
	enum class en_PlayerJobType : int16
	{		
		PLAYER_MELEE_JOB,
		PLAYER_MAGIC_JOB,
		PLAYER_MAGIC_CANCEL_JOB
	};

	struct st_PlayerJob
	{
		en_PlayerJobType Type;		
		CGameServerMessage* Message = nullptr;
	};	

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
	CInventoryManager _InventoryManager;
	
	CSkillBox _SkillBox;
	CQuickSlotManager _QuickSlotManager;

	st_Experience _Experience;	

	CLockFreeQue<st_PlayerJob*> _PlayerJobQue;

	// 시야 범위 오브젝트
	vector<st_FieldOfViewInfo> _FieldOfViewInfos;	

	CPlayer();	
	~CPlayer();		

	virtual void Update() override;

	virtual bool OnDamaged(CGameObject* Attacker, int32 Damage) override;
	virtual void OnDead(CGameObject* Killer) override;	

	void Init();	

	virtual void PositionReset() override;
	void PlayerJobQueProc();
protected:
	virtual void UpdateMove();	
	virtual void UpdateAttack();
	virtual void UpdateSpell();	
};