#pragma once
#include "GameObject.h"

class CPlayer;

class CMonster : public CGameObject
{
private:
	//---------------
	// Ÿ��
	//---------------
	CPlayer* _Target;

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
protected:
	//------------------------
	// Idle ���� Update
	//------------------------
	virtual void UpdateIdle();
	//------------------------
	// Moving ���� Update
	//------------------------
	virtual void UpdateMoving();
	//------------------------
	// Attack ���� Update
	//------------------------
	virtual void UpdateAttack();
	//------------------------
	// Dead ���� Update
	//------------------------
	virtual void UpdateDead();
public:
	// ���� ������ ��Ʈ Id
	int32 _DataSheetId;

	CMonster();
	~CMonster();

	void Init(int32 DataSheetId);

	virtual void Update() override;
	virtual void OnDamaged(CGameObject* Attacker, int32 Damage) override;
	virtual void OnDead(CGameObject* Killer) override;
	
	void BroadCastPacket(en_PACKET_TYPE PacketType);
};

