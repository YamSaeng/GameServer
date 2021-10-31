#pragma once
#include "GameObject.h"
#include "Data.h"

class CPlayer;

class CMonster : public CGameObject
{
public:
	// 몬스터 데이터 시트 Id
	int32 _DataSheetId;

	// 몬스터 죽이면 얻는 DPPoint
	int16 _GetDPPoint;	
	// 몬스터 죽이면 얻는 ExpPoint
	int32 _GetExpPoint;

	CMonster();
	virtual ~CMonster();	

	virtual void Update() override;
	virtual void OnDamaged(CGameObject* Attacker, int32 Damage) override;
	// 몬스터 초기화
	virtual void Init(st_Vector2Int SpawnPosition);	
protected:
	//--------------------------
	// Idle 상태에서 Search 거리
	//--------------------------	
	int32 _SearchCellDistance;
	//--------------------------
	// Moving 상태에서 추격 거리
	//--------------------------
	int32 _ChaseCellDistance;

	//-----------------
	// 공격 거리
	//-----------------
	int32 _AttackRange;

	//--------------------------------------------
	// SpawnIdle 상태에서 Idle 상태로 돌아갈 Tick
	//--------------------------------------------
	uint64 _SpawnIdleTick;
	//------------------------------------
	// Idle 상태에서 Search를 실행할 Tick
	//------------------------------------
	uint64 _SearchTick;
	//------------------------------------
	// 탐색 속도
	//------------------------------------
	uint64 _SearchTickPoint;
	//------------------------------------
	// Patrol 상태에서 Patrol를 실행할 Tick
	//------------------------------------
	uint64 _PatrolTick;
	//------------------------------------
	// 정찰 속도
	//------------------------------------
	uint64 _PatrolTickPoint;
	//-------------------------------------
	// Moving 상태에서 Chase를 실행할 Tick
	//-------------------------------------
	uint64 _MoveTick;

	//--------------------------------------
	// Attack 상태에서 Attack을 실행할 Tick
	//--------------------------------------
	uint64 _AttackTick;
	//------------------------------------
	// 공격 속도
	//------------------------------------
	uint64 _AttackTickPoint;	

	//-------------------------------------
	// 몬스터가 정찰할 위치
	//-------------------------------------
	vector<st_Vector2Int> _PatrolPositions;	

	//------------------------
	// Idle 상태 Update
	//------------------------
	virtual void UpdateIdle();
	//------------------------
	// Patrol 상태 Update
	//------------------------
	virtual void UpdatePatrol();
	//------------------------
	// Moving 상태 Update
	//------------------------
	virtual void UpdateMoving();
	//--------------------------------------
	// ReturnSpawnPosition 상태 Update
	//--------------------------------------
	virtual void UpdateReturnSpawnPosition();
	//------------------------
	// Attack 상태 Update
	//------------------------
	virtual void UpdateAttack();
	//------------------------
	// Dead 상태 Update
	//------------------------
	virtual void UpdateDead() = 0;	
	//------------------------
	// Spawn Idle 상태 Update
	//------------------------
	virtual void UpdateSpawnIdle();
};

