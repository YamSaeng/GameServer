#pragma once
#include "Creature.h"
#include "MonsterSkillBox.h"
#include "Data.h"

class CPlayer;

class CMonster : public CCreature
{
public:
	// ���� ������ ��Ʈ Id
	int32 _DataSheetId;

	// ���� ���̸� ��� ExpPoint
	int32 _GetExpPoint;	

	en_MonsterState _MonsterState;	    
	
	// ���� ��ų
	CMonsterSkillBox _MonsterSkillBox;

	CMonster();
	virtual ~CMonster();	

	virtual void Update() override;

	virtual void PositionReset() override;
	virtual bool OnDamaged(CGameObject* Attacker, int32 Damage) override;
	
	virtual void Start() override;
	virtual void End() override;

	CGameObject* GetTarget();
protected:		

	// ���� ���� ��ų
	CSkill* _CurrentSkill;	

	// ��ǥ��
	CGameObject* _Target;			

	// ���� ��ġ	
	Vector2 _PatrolPoint;

	// �̵� ��ġ	
	Vector2 _MovePoint;
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
	vector<Vector2Int> _PatrolPositions;			
		
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
	// Rooting ���� Update
	//------------------------
	virtual void UpdateRooting() override;
	//------------------------
	// Dead ���� Update
	//------------------------
	virtual void UpdateDead() override;

	void Move();	

	bool TargetAttackCheck(float CheckDistance);
};

