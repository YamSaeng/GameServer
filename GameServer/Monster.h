#pragma once
#include "GameObject.h"
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

	//-------------------------------------
	// ���� �þ߹��� �÷��̾� ���
	//-------------------------------------
	vector<CPlayer*> _FieldOfViewPlayers;
	
	CMonster();
	virtual ~CMonster();	

	virtual void Update() override;

	virtual void PositionReset() override;
	virtual bool OnDamaged(CGameObject* Attacker, int32 Damage) override;
	// ���� �ʱ�ȭ
	virtual void Init(st_Vector2Int SpawnPosition);
protected:
	CGameObject* _Target;
	//---------------------
	// ���� ��ġ
	//---------------------
	st_Vector2 _PatrolPoint;
	//---------------------
	// �̵� ��ġ
	//---------------------
	st_Vector2 _MovePoint;

	//--------------------------
	// Idle ���¿��� Search �Ÿ�
	//--------------------------	
	int32 _SearchCellDistance;
	//--------------------------
	// Moving ���¿��� �߰� �Ÿ�
	//--------------------------
	int32 _ChaseCellDistance;

	//-----------------
	// ���� �Ÿ�
	//-----------------
	int32 _AttackRange;

	//--------------------------------------------
	// SpawnIdle ���¿��� Idle ���·� ���ư� Tick
	//--------------------------------------------
	uint64 _SpawnIdleTick;
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

	//--------------------------------------
	// Attack ���¿��� Attack�� ������ Tick
	//--------------------------------------
	uint64 _DefaultAttackTick;
	//------------------------------------
	// ���� �ӵ�
	//------------------------------------
	uint64 _AttackTickPoint;	

	//-------------------------------------
	// ���Ͱ� ������ ��ġ
	//-------------------------------------
	vector<st_Vector2Int> _PatrolPositions;			
		
	void SelectTarget();

	CGameObject* FindTarget();
	//------------------------
	// Spawn Idle ���� Update
	//------------------------
	virtual void UpdateSpawnIdle();
	//------------------------
	// Idle ���� Update
	//------------------------
	virtual void UpdateIdle();
	//------------------------
	// Patrol ���� Update
	//------------------------
	void ReadyPatrol();
	virtual void UpdatePatrol();
	//------------------------
	// Moving ���� Update
	//------------------------
	void ReadMoving();
	virtual void UpdateMoving();		
	//--------------------------------------
	// ReturnSpawnPosition ���� Update
	//--------------------------------------
	virtual void UpdateReturnSpawnPosition();
	//------------------------
	// Attack ���� Update
	//------------------------
	virtual void UpdateAttack();
	//------------------------
	// Spell ���� Update
	//------------------------
	virtual void UpdateSpell();
	//------------------------
	// Dead ���� Update
	//------------------------
	virtual void UpdateDead();			

	void Move();

	void SendMonsterChangeObjectState();		

	void TargetAttackCheck();
};

