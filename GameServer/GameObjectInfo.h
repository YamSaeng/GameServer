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
	int64 ItemDBId;				// ������ DB�� ����Ǿ� �ִ� ID		
	en_ItemType ItemType;		// ������ Ÿ��
	en_ConsumableType ItemConsumableType;	// �Һ�� ����������
	wstring ItemName;			// ������ �̸�
	int16 ItemCount;			// ����
	wstring ThumbnailImagePath; // �̹��� ���
	bool IsEquipped;			// �������� ������ �� �ִ���	
	int8 SlotIndex;				// ���� ��ȣ
};

struct st_SkillInfo
{
	en_SkillType _SkillType; // ��ų ����
	int8 _SkillLevel;		 // ��ų ����
	wstring _SkillName;		 // ��ų �̸�
	int32 _SkillCoolTime;	 // ��ų ��Ÿ��
	int8 _QuickSlotBarIndex; // �����Թ� �ε��� ( � �����Թ� ���� )
	int8 _QuickSlotBarItemIndex; // �����Թ� ������ �ε��� ( �����Թ��� ���°�� ���ϴ��� )
	wstring _SkillImagePath; // ��ų �̹��� ���
};