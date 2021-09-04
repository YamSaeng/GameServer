#pragma once
#include "GameObject.h"
#include "Data.h"

class CPlayer;

class CMonster : public CGameObject
{
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

	//------------------------------------
	// Idle ���¿��� Search�� ������ Tick
	//------------------------------------
	uint64 _NextSearchTick;
	//-------------------------------------
	// Moving ���¿��� Chase�� ������ Tick
	//-------------------------------------
	uint64 _NextMoveTick;

	//--------------------------------------
	// Attack ���¿��� Attack�� ������ Tick
	//--------------------------------------
	uint64 _AttackTick;

	//------------------------
	// Idle ���� Update
	//------------------------
	virtual void UpdateIdle() = 0;
	//------------------------
	// Moving ���� Update
	//------------------------
	virtual void UpdateMoving() = 0;
	//------------------------
	// Attack ���� Update
	//------------------------
	virtual void UpdateAttack() = 0;
	//------------------------
	// Dead ���� Update
	//------------------------
	virtual void UpdateDead() = 0;
	
public:
	// ���� ������ ��Ʈ Id
	int32 _DataSheetId;

	CMonster();
	virtual ~CMonster();

	void Init(int32 DataSheetId);

	virtual void Update() override;
	virtual void OnDamaged(CGameObject* Attacker, int32 Damage) override;
};

