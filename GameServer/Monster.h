#pragma once
#include "GameObject.h"
#include "MonsterSkillBox.h"
#include "Data.h"

class CPlayer;

class CMonster : public CGameObject
{
public:
	// ���� ������ ��Ʈ Id
	int32 _DataSheetId;

	// ���� ���̸� ��� DPPoint
	int16 _GetDPPoint;	
	// ���� ���̸� ��� ExpPoint
	int32 _GetExpPoint;	

	en_MonsterState _MonsterState;	    
	
	CMonster();
	virtual ~CMonster();	

	virtual void Update() override;

	virtual void PositionReset() override;
	virtual bool OnDamaged(CGameObject* Attacker, int32 Damage) override;
	
	virtual void Start() override;
	virtual void End() override;

	CGameObject* GetTarget();
protected:	
	// ���� ��ų
	CMonsterSkillBox _MonsterSkillBox;

	// ���� ���� ��ų
	CSkill* _CurrentSkill;	

	// ��ǥ��
	CGameObject* _Target;			

	// ���� ��ġ	
	st_Vector2 _PatrolPoint;	

	// �̵� ��ġ	
	st_Vector2 _MovePoint;	
	//------------------------------------
	// Idle ���¿��� Search�� ������ Tick
	//------------------------------------
	uint64 _SearchTick;
	//------------------------------------
	// Ž�� �ӵ�
	//------------------------------------
	uint64 _SearchTickPoint;
	//------------------------------------
	// Patrol ���¿��� Patrol�� ������ Tick
	//------------------------------------
	uint64 _PatrolTick;
	//------------------------------------
	// ���� �ӵ�
	//------------------------------------
	uint64 _PatrolTickPoint;
	//-------------------------------------
	// Moving ���¿��� Chase�� ������ Tick
	//-------------------------------------
	uint64 _MoveTick;

	//------------------------------------
	// ���� �ӵ�
	//------------------------------------
	uint64 _AttackTickPoint;		

	//-------------------------------------
	// ���Ͱ� ������ ��ġ
	//-------------------------------------
	vector<st_Vector2Int> _PatrolPositions;			
		
	//---------------------------------------------------------------------------
	// ��׷� ��� �� ��Ʈ��ũ�� ���� Ÿ��, ���� ���� Ÿ���� ��׷� ��Ͽ��� ����
	//---------------------------------------------------------------------------
	void AggroTargetListCheck();	
	//------------------------------------------------------------
	// ��׷� ��� �� ��׷� ��ġ�� ���� ���� ����� Ÿ������ ����
	//------------------------------------------------------------
	void SelectTarget();
	//------------------------------------------------------------
	// ������ ��ǥ���� �˻�
	//------------------------------------------------------------
	void SelectTargetCheck();

	//------------------------------------------------------
	// ���� ��� ã��
	//------------------------------------------------------
	void FindTarget();
	//------------------------
	// Spawn Ready ���� Update
	//------------------------
	virtual void UpdateSpawnReady() override;
	//------------------------
	// Spawn Idle ���� Update
	//------------------------
	virtual bool UpdateSpawnIdle() override;
	//------------------------
	// Idle ���� Update
	//------------------------
	virtual void UpdateIdle() override;
	//------------------------
	// Patrol ���� Update
	//------------------------
	void ReadyPatrol();
	virtual void UpdatePatrol() override;
	//------------------------
	// Moving ���� Update
	//------------------------	
	void ReadMoving();
	virtual void UpdateMoving() override;		
	//--------------------------------------
	// ReturnSpawnPosition ���� Update
	//--------------------------------------
	virtual void UpdateReturnSpawnPosition() override;
	//------------------------
	// Attack ���� Update
	//------------------------
	virtual void UpdateAttack() override;
	//------------------------
	// Spell ���� Update
	//------------------------
	virtual void UpdateSpell() override;	
	//------------------------
	// Dead ���� Update
	//------------------------
	virtual void UpdateDead() override;

	void Move();	

	bool TargetAttackCheck(float CheckDistance);
};

