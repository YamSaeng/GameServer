#pragma once

enum class en_GameObjectType : int16
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

enum class en_MoveDir : int8
{
	UP,
	DOWN,
	LEFT,
	RIGHT
};

enum class en_CreatureState : int8
{
	IDLE,
	MOVING,
	ATTACK,
	SPELL,
	DEAD,
};

enum class en_AttackRange : int8
{
	NORMAL_ATTACK,
	FORWARD_ATTACK,
	AROUND_ONE_ATTACK,
	MAGIC_ATTACK,
};

enum class en_AttackType : int16
{
	NONE_ATTACK = -1,
	MELEE_PLAYER_NORMAL_ATTACK,
	MELEE_PLAYER_CHOHONE_ATTACK,
	MELEE_PLAYER_SHAEHONE_ATTACK,
	MELEE_PLAYER_AROUND_ATTACK,
	MAGIC_PLAYER_NORMAL_ATTACK,
	MAGIC_PLAYER_FIRE_ATTACK,
	SLIME_NORMAL_ATTACK,
	BEAR_NORMAL_ATTACK
};

enum class en_MessageType : int8
{
	CHATTING,
	SYSTEM
};

enum class en_StateChange : int16
{
	MOVE_TO_STOP,
	SPELL_TO_IDLE,
};

enum class en_ObjectNetworkState : int8
{
	READY,
	LIVE,
	LEAVE
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
	en_GameObjectType OwnerObjectType;
	int8 PlayerSlotIndex;
};

struct st_Color
{
	int8 _Red;
	int8 _Green;
	int8 _Blue;

	st_Color() {}
	st_Color(int8 Red, int8 Green, int8 Blue)
	{
		_Red = Red;
		_Green = Green;
		_Blue = Blue;
	}

	static st_Color Red() { return st_Color(127, 0, 0); }
	static st_Color Green() { return st_Color(0, 127, 0); }
	static st_Color Blue() { return st_Color(0, 0, 127); }
	static st_Color White() { return st_Color(127, 127, 127); }
};