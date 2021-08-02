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

struct st_PositionInfo
{
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
};

struct st_PlayerObjectInfo : public st_GameObjectInfo
{

};

class CGameObject
{
private:
protected:
	st_GameObjectInfo _GameObjectInfo;
	CChannel _Channel;
public:
	CGameObject() {};
	CGameObject(st_GameObjectInfo GameObjectInfo);
	~CGameObject();

	virtual void Update();
	virtual void OnDamaged(CGameObject Attacker, int32 Damage);
	virtual void OnDead(CGameObject Killer);

	st_PositionInfo GetPositionInfo();
};

