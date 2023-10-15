#pragma once
#include "Creature.h"
#include "MonsterSkillBox.h"
#include "Data.h"

class CPlayer;

class CMonster : public CCreature
{
public:
	// 몬스터 데이터 시트 Id
	int32 _DataSheetId;

	// 몬스터 죽이면 얻는 ExpPoint
	int32 _GetExpPoint;	

	en_MonsterState _MonsterState;	    
	
	// 몬스터 스킬
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

	// 시전 중인 스킬
	CSkill* _CurrentSkill;	

	// 목표물
	CGameObject* _Target;			

	// 정찰 위치	
	Vector2 _PatrolPoint;

	// 이동 위치	
	Vector2 _MovePoint;
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

	//------------------------------------
	// 공격 속도
	//------------------------------------
	uint64 _AttackTickPoint;		

	//-------------------------------------
	// 몬스터가 정찰할 위치
	//-------------------------------------
	vector<Vector2Int> _PatrolPositions;			
		
	//---------------------------------------------------------------------------
	// 어그로 목록 중 네트워크가 끊긴 타겟, 죽음 상태 타겟을 어그로 목록에서 정리
	//---------------------------------------------------------------------------
	void AggroTargetListCheck();	
	//------------------------------------------------------------
	// 어그로 목록 중 어그로 수치가 가장 높은 대상을 타겟으로 정함
	//------------------------------------------------------------
	void SelectTarget();
	//------------------------------------------------------------
	// 선택한 목표물을 검사
	//------------------------------------------------------------
	void SelectTargetCheck();

	//------------------------------------------------------
	// 추적 대상 찾기
	//------------------------------------------------------
	void FindTarget();
	//------------------------
	// Spawn Ready 상태 Update
	//------------------------
	virtual void UpdateSpawnReady() override;
	//------------------------
	// Spawn Idle 상태 Update
	//------------------------
	virtual bool UpdateSpawnIdle() override;
	//------------------------
	// Idle 상태 Update
	//------------------------
	virtual void UpdateIdle() override;
	//------------------------
	// Patrol 상태 Update
	//------------------------
	void ReadyPatrol();
	virtual void UpdatePatrol() override;
	//------------------------
	// Moving 상태 Update
	//------------------------	
	void ReadMoving();
	virtual void UpdateMoving() override;		
	//--------------------------------------
	// ReturnSpawnPosition 상태 Update
	//--------------------------------------
	virtual void UpdateReturnSpawnPosition() override;
	//------------------------
	// Attack 상태 Update
	//------------------------
	virtual void UpdateAttack() override;
	//------------------------
	// Spell 상태 Update
	//------------------------
	virtual void UpdateSpell() override;	
	//------------------------
	// Rooting 상태 Update
	//------------------------
	virtual void UpdateRooting() override;
	//------------------------
	// Dead 상태 Update
	//------------------------
	virtual void UpdateDead() override;

	void Move();	

	bool TargetAttackCheck(float CheckDistance);
};

