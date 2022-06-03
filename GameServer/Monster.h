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
	
	virtual void Start() override;
	virtual void End() override;
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
	float _AttackRange;
	
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

	CGameObject* FindTarget();
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
	//----------------------------
	// Ready Dead ���� Update
	//----------------------------
	virtual void UpdateReadyDead() override;
	//------------------------
	// Dead ���� Update
	//------------------------
	virtual void UpdateDead() override;

	void Move();

	void SendMonsterChangeObjectState();		

	bool TargetAttackCheck();
};

