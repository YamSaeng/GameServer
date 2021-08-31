#pragma once

enum en_GameObjectType
{
	NORMAL,
	MELEE_PLAYER,
	MAGIC_PLAYER,
	SLIME,
	BEAR,
	WEAPON,
	SLIME_GEL,
	LEATHER,
	BRONZE_COIN,
	SLIVER_COIN,
	GOLD_COIN
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
	SPELL,
	DEAD,
};

enum en_AttackRange
{
	NORMAL_ATTACK,
	FORWARD_ATTACK,
	AROUND_ONE_ATTACK,
	MAGIC_ATTACK,
};

enum en_AttackType
{
	NONE_ATTACK = -1,
	MELEE_PLAYER_NORMAL_ATTACK,
	MELEE_PLAYER_CHOHONE_ATTACK,
	MELEE_PLAYER_SHAEHONE_ATTACK,
	MELEE_PLAYER_RANGE_ATTACK,
	MAGIC_PLAYER_NORMAL_ATTACK,
	MAGIC_PLAYER_FIRE_ATTACK,
	SLIME_NORMAL_ATTACK,
	BEAR_NORMAL_ATTACK
};

enum en_MessageType
{
	CHATTING,
	SYSTEM
};

enum en_StateChange
{
	MOVE_TO_STOP,
	SPELL_TO_IDLE,
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
	int16 CriticalPoint;
	float Speed;
};

struct st_GameObjectInfo
{
	int64 ObjectId;
	wstring ObjectName;
	st_PositionInfo ObjectPositionInfo;
	st_StatInfo ObjectStatInfo;
	en_GameObjectType ObjectType;
	int64 OwnerObjectId;
	int8 PlayerSlotIndex;
};

enum en_ObjectNetworkState
{
	READY,
	LIVE,
	LEAVE
};