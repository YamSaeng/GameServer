#pragma once

#include "Channel.h"
#include "CommonProtocol.h"
#include "GameObjectInfo.h"

class CGameObject
{
private:
public:
	bool _IsSendPacketTarget;

	int32 _ObjectManagerArrayIndex;
	int32 _ChannelArrayIndex;
	en_ObjectNetworkState _NetworkState;
	st_GameObjectInfo _GameObjectInfo;	
	CChannel* _Channel;	

	// 선택한 대상
	CGameObject* _SelectTarget;

	//---------------------------
	// 오브젝트가 스폰될 위치
	//---------------------------
	st_Vector2Int _SpawnPosition;

	// 시야 범위
	int8 _FieldOfViewDistance;	

	CGameObject();
	CGameObject(st_GameObjectInfo GameObjectInfo);
	virtual ~CGameObject();

	virtual void Update();
	virtual bool OnDamaged(CGameObject* Attacker, int32 DamagePoint);
	virtual void OnHeal(CGameObject* Healer, int32 HealPoint);
	virtual void OnDead(CGameObject* Killer);

	st_Vector2 PositionCheck(st_Vector2Int& CheckPosition);
	virtual void PositionReset();

	st_PositionInfo GetPositionInfo();

	//--------------------------------------------
	// 현재 좌표 기준으로 st_Vector2Int를 반환한다.
	//--------------------------------------------
	st_Vector2Int GetCellPosition();

	//------------------------------------------------
	// 방향값을 받아서 앞쪽에 있는 위치를 반환한다.
	//------------------------------------------------
	st_Vector2Int GetFrontCellPosition(en_MoveDir Dir,int8 Distance);
	
	//------------------------------------------------------------------------------------
	// 내 주위 Distance안에 있는 위치들의 값을 반환한다.
	//------------------------------------------------------------------------------------
	vector<st_Vector2Int> GetAroundCellPositions(st_Vector2Int CellPosition, int8 Distance);
	
	void SetTarget(CGameObject* Target);
	CGameObject* GetTarget();
protected:
	//---------------
	// 타겟
	//---------------
	CGameObject* _Target;	

	//-------------------------
	// 재생력 Tick
	//-------------------------
	uint64 _NatureRecoveryTick;	

	//---------------------------
	// 주위 시야 오브젝트 탐색 틱
	//---------------------------
	uint64 _FieldOfViewUpdateTick;

	void BroadCastPacket(en_PACKET_TYPE PacketType, bool CanMove = true);
};

