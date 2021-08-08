#pragma once

#include "Channel.h"

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
	st_PositionInfo ObjectPositionInfo;
	st_StatInfo ObjectStatInfo;
	en_GameObjectType ObjectType;
};

struct st_PlayerObjectInfo : public st_GameObjectInfo
{

};

class CGameObject
{
private:
protected:	
public:
	st_GameObjectInfo _GameObjectInfo;
	CChannel* _Channel;	

	CGameObject();
	CGameObject(st_GameObjectInfo GameObjectInfo);
	~CGameObject();

	virtual void Update();
	virtual void OnDamaged(CGameObject Attacker, int32 Damage);
	virtual void OnDead(CGameObject Killer);

	st_PositionInfo GetPositionInfo();
};

