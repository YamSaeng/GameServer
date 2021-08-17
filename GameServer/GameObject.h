#pragma once

#include "Channel.h"
#include "CommonProtocol.h"

enum en_GameObjectType
{
	NORMAL,
	PLAYER,
	MONSTER,
};

enum en_MoveDir
{
	UP,
	DOWN,
	LEFT,
	RIGHT
};

enum en_CreatureState
{
	IDLE,
	MOVING,
	ATTACK,
	DEAD,
};

struct st_PositionInfo
{
	en_CreatureState State;
	int32 PositionX;
	int32 PositionY;
	en_MoveDir MoveDir;
};

struct st_StatInfo
{
	int32 Level;
	int32 HP;	
	int32 MaxHP;
	int32 Attack;
	float Speed;
};

struct st_GameObjectInfo
{
	int64 ObjectId;	
	wstring ObjectName;
	st_PositionInfo ObjectPositionInfo;
	st_StatInfo ObjectStatInfo;
	en_GameObjectType ObjectType;
};

enum en_ObjectNetworkState
{
	READY,
	LIVE,
	LEAVE
};

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
	st_Vector2Int GetFrontCellPosition(en_MoveDir Dir);

	en_MoveDir GetDirectionFromVector(st_Vector2Int DirectionVector);
};

