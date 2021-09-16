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
	GOLD_COIN,
	SKILL_BOOK	
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

enum class en_Inventory : int8
{
	INVENTORY_SIZE = 30
};

enum class en_ItemType : int16
{
	ITEM_TYPE_NONE = 0,

	ITEM_TYPE_WEAPON_SWORD = 1,

	ITEM_TYPE_ARMOR_ARMOR = 100,
	ITEM_TYPE_ARMOR_HELMET = 101,

	ITEM_TYPE_CONSUMABLE_POTION = 200,
	ITEM_TYPE_SKILL_BOOK = 300,

	ITEM_TYPE_LEATHER = 2000,
	ITEM_TYPE_SLIMEGEL = 2001,
	ITEM_TYPE_BRONZE_COIN = 2002,
	ITEM_TYPE_SLIVER_COIN = 2003,
	ITEM_TYPE_GOLD_COIN = 2004
};

enum class en_SkillType : int16
{
	SKILL_TYPE_NONE = 0,	
	SKILL_KNIGHT_CHOHONE,
	SKILL_KNIGHT_SHAEHONE,
	SKILL_SHAMAN_FIRE	
};

enum class en_ConsumableType : int16
{
	NONE,
	POTION,
	SKILL_BOOK
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
	int32 MinAttackDamage;
	int32 MaxAttackDamage;	
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
	int16 _Red;
	int16 _Green;
	int16 _Blue;

	st_Color() {}
	st_Color(int16 Red, int16 Green, int16 Blue)
	{
		_Red = Red;
		_Green = Green;
		_Blue = Blue;
	}

	static st_Color Red() { return st_Color(255, 0, 0); }
	static st_Color Green() { return st_Color(0, 255, 0); }
	static st_Color Blue() { return st_Color(0, 0, 255); }
	static st_Color Yellow() { return st_Color(255, 212, 255); }
	static st_Color White() { return st_Color(255, 255, 255); }
};

struct st_ItemInfo
{
	int64 ItemDBId;				// 아이템 DB에 저장되어 있는 ID		
	en_ItemType ItemType;		// 아이템 타입
	en_ConsumableType ItemConsumableType;	// 소비용 아이템인지
	wstring ItemName;			// 아이템 이름
	int16 ItemCount;			// 개수
	wstring ThumbnailImagePath; // 이미지 경로
	bool IsEquipped;			// 아이템을 착용할 수 있는지	
	int8 SlotIndex;				// 슬롯 번호
};

struct st_SkillInfo
{
	en_SkillType _SkillType; // 스킬 종류
	int8 _SkillLevel;		 // 스킬 레벨
	wstring _SkillName;		 // 스킬 이름
	int32 _SkillCoolTime;	 // 스킬 쿨타임
	int8 _QuickSlotBarIndex; // 퀵슬롯바 인덱스 ( 어떤 퀵슬롯바 인지 )
	int8 _QuickSlotBarItemIndex; // 퀵슬롯바 아이템 인덱스 ( 퀵슬롯바의 몇번째에 속하는지 )
	wstring _SkillImagePath; // 스킬 이미지 경로
};