#pragma once
#include "GameObject.h"

class CPlayer;

class CMonster : public CGameObject
{
private:
	//---------------
	// 타겟
	//---------------
	CPlayer* _Target;

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

	//------------------------------------
	// Idle 상태에서 Search를 실행할 Tick
	//------------------------------------
	uint64 _NextSearchTick;
	//-------------------------------------
	// Moving 상태에서 Chase를 실행할 Tick
	//-------------------------------------
	uint64 _NextMoveTick;

	//--------------------------------------
	// Attack 상태에서 Attack을 실행할 Tick
	//--------------------------------------
	uint64 _AttackTick;
protected:
	//------------------------
	// Idle 상태 Update
	//------------------------
	virtual void UpdateIdle();
	//------------------------
	// Moving 상태 Update
	//------------------------
	virtual void UpdateMoving();
	//------------------------
	// Attack 상태 Update
	//------------------------
	virtual void UpdateAttack();
	//------------------------
	// Dead 상태 Update
	//------------------------
	virtual void UpdateDead();
public:
	// 몬스터 데이터 시트 Id
	int32 _DataSheetId;

	CMonster();
	~CMonster();

	void Init(int32 DataSheetId);

	virtual void Update() override;
	virtual void OnDamaged(CGameObject* Attacker, int32 Damage) override;
	virtual void OnDead(CGameObject* Killer) override;
	
	void BroadCastPacket(en_PACKET_TYPE PacketType);
};

