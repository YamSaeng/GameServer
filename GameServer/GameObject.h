#pragma once

#include "Channel.h"
#include "CommonProtocol.h"
#include "GameObjectInfo.h"

class CGameObject
{
private:
public:
	en_ObjectNetworkState _NetworkState;
	st_GameObjectInfo _GameObjectInfo;
	CChannel* _Channel;	

	// ������ ���
	CGameObject* _SelectTarget;

	//---------------------------
	// ������Ʈ�� ������ ��ġ
	//---------------------------
	st_Vector2Int _SpawnPosition;

	CGameObject();
	CGameObject(st_GameObjectInfo GameObjectInfo);
	virtual ~CGameObject();

	virtual void Update();
	virtual void OnDamaged(CGameObject* Attacker, int32 DamagePoint);
	virtual void OnHeal(CGameObject* Healer, int32 HealPoint);
	virtual void OnDead(CGameObject* Killer);

	st_PositionInfo GetPositionInfo();

	//--------------------------------------------
	// ���� ��ǥ �������� st_Vector2Int�� ��ȯ�Ѵ�.
	//--------------------------------------------
	st_Vector2Int GetCellPosition();

	//------------------------------------------------
	// ���Ⱚ�� �޾Ƽ� ���ʿ� �ִ� ��ġ�� ��ȯ�Ѵ�.
	//------------------------------------------------
	st_Vector2Int GetFrontCellPosition(en_MoveDir Dir,int8 Distance);
	
	//------------------------------------------------------------------------------------
	// �� ���� Distance�ȿ� �ִ� ��ġ���� ���� ��ȯ�Ѵ�.
	//------------------------------------------------------------------------------------
	vector<st_Vector2Int> GetAroundCellPositions(st_Vector2Int CellPosition, int8 Distance);
	
	void SetTarget(CGameObject* Target);
	CGameObject* GetTarget();
protected:
	//---------------
	// Ÿ��
	//---------------
	CGameObject* _Target;	

	void BroadCastPacket(en_PACKET_TYPE PacketType);
};

