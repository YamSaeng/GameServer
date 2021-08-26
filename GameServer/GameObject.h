#pragma once

#include "Channel.h"
#include "CommonProtocol.h"
#include "GameObjectInfo.h"

class CGameObject
{
private:
protected:	
public:
	en_ObjectNetworkState _NetworkState;
	st_GameObjectInfo _GameObjectInfo;
	CChannel* _Channel;	

	CGameObject();
	CGameObject(st_GameObjectInfo GameObjectInfo);
	~CGameObject();

	virtual void Update();
	virtual void OnDamaged(CGameObject* Attacker, int32 Damage);
	virtual void OnDead(CGameObject* Killer);

	st_PositionInfo GetPositionInfo();

	//--------------------------------------------
	// 현재 좌표 기준으로 st_Vector2Int를 반환한다.
	//--------------------------------------------
	st_Vector2Int GetCellPosition();

	//------------------------------------------------
	// 방향값을 받아서 앞쪽에 있는 위치를 반환한다.
	//------------------------------------------------
	st_Vector2Int GetFrontCellPosition(en_MoveDir Dir,int8 Distance);

	vector<st_Vector2Int> GetAroundCellPosition(st_Vector2Int CellPosition, int8 Distance);

	en_MoveDir GetDirectionFromVector(st_Vector2Int DirectionVector);
};

