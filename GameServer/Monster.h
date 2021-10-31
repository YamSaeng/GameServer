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

	CMonster();
	virtual ~CMonster();	

	virtual void Update() override;
	virtual void OnDamaged(CGameObject* Attacker, int32 Damage) override;
	// ���� �ʱ�ȭ
	virtual void Init(st_Vector2Int SpawnPosition);	
protected:
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
	uint64 _AttackTick;
	//------------------------------------
	// ���� �ӵ�
	//------------------------------------
	uint64 _AttackTickPoint;	

	//-------------------------------------
	// ���Ͱ� ������ ��ġ
	//-------------------------------------
	vector<st_Vector2Int> _PatrolPositions;	

	//------------------------
	// Idle ���� Update
	//------------------------
	virtual void UpdateIdle();
	//------------------------
	// Patrol ���� Update
	//------------------------
	virtual void UpdatePatrol();
	//------------------------
	// Moving ���� Update
	//------------------------
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
	// Dead ���� Update
	//------------------------
	virtual void UpdateDead() = 0;	
	//------------------------
	// Spawn Idle ���� Update
	//------------------------
	virtual void UpdateSpawnIdle();
};

