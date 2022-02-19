#pragma once
#include "Type.h"

enum class en_CreatureState : int8
{
	SPAWN_IDLE,
	IDLE,
	PATROL,
	MOVING,
	STOP,
	RETURN_SPAWN_POSITION,
	ATTACK,
	SPELL,
	DEAD,
	STUN,
	PUSH_AWAY,
	ROOT
};

enum class en_MoveDir : int8
{
	UP,
	DOWN,
	LEFT,
	RIGHT
};

enum class en_GameObjectType : int16
{
	NORMAL,

	OBJECT_PLAYER,
	OBJECT_WARRIOR_PLAYER,
	OBJECT_MAGIC_PLAYER,
	OBJECT_TAIOIST_PLAYER,
	OBJECT_THIEF_PLAYER,
	OBJECT_ARCHER_PLAYER,

	OBJECT_MONSTER,
	OBJECT_SLIME,
	OBJECT_BEAR,

	OBJECT_ENVIRONMENT,
	OBJECT_STONE,
	OBJECT_TREE,

	OBJECT_ITEM,
	OBJECT_ITEM_WEAPON,
	OBJECT_ITEM_WEAPON_WOOD_SWORD,

	OBJECT_ITEM_ARMOR,
	OBJECT_ITEM_ARMOR_WOOD_ARMOR,

	OBJECT_ITEM_ARMOR_LEATHER_HELMET,
	OBJECT_ITEM_ARMOR_LEATHER_BOOT,

	OBJECT_ITEM_CONSUMABLE,
	OBJECT_ITEM_CONSUMABLE_SKILL_BOOK,
	OBJECT_ITEM_CONSUMABLE_HEAL_POTION_SMALL,

	OBJECT_ITEM_MATERIAL,
	OBJECT_ITEM_MATERIAL_SLIME_GEL,
	OBJECT_ITEM_MATERIAL_LEATHER,
	OBJECT_ITEM_MATERIAL_BRONZE_COIN,
	OBJECT_ITEM_MATERIAL_SLIVER_COIN,
	OBJECT_ITEM_MATERIAL_GOLD_COIN,
	OBJECT_ITEM_MATERIAL_WOOD_LOG,
	OBJECT_ITEM_MATERIAL_STONE,
	OBJECT_ITEM_MATERIAL_WOOD_FLANK,
	OBJECT_ITEM_MATERIAL_YARN,

	OBJECT_PLAYER_DUMMY = 32000
};

enum class en_SkillLargeCategory : int8
{
	SKILL_LARGE_CATEGORY_NONE = 0,
	SKILL_LARGE_CATEOGRY_PUBLIC,

	SKILL_LARGE_CATEGORY_WARRIOR,

	SKILL_LARGE_CATEGORY_SHMAN,

	SKILL_LARGE_CATEGORY_TAOIST,

	SKILL_LARGE_CATEGORY_THIEF,

	SKILL_LARGE_CATEGORY_ARCHER,

	SKILL_LARGE_CATEGORY_MONSTER_MELEE,
	SKILL_LARGE_CATEGORY_MONSTER_MAGIC
};

enum class en_SkillMediumCategory : int8
{
	SKILL_MEDIUM_CATEGORY_NONE = 0,
	SKILL_MEDIUM_CATEGORY_PUBLIC_ATTACK,
	SKILL_MEDIUM_CATEGORY_PUBLIC_HEAL,
	SKILL_MEDIUM_CATEGORY_PUBLIC_BUF,

	SKILL_MEDIUM_CATEGORY_WARRIOR_ATTACK,
	SKILL_MEDIUM_CATEGORY_WARRIOR_HEAL,
	SKILL_MEDIUM_CATEGORY_WARRIOR_BUF,

	SKILL_MEDIUM_CATEGORY_SHMAN_ATTACK,
	SKILL_MEDIUM_CATEGORY_SHMAN_HEAL,
	SKILL_MEDIUM_CATEGORY_SHMAN_BUF,

	SKILL_MEDIUM_CATEGORY_TAOIST_ATTACK,
	SKILL_MEDIUM_CATEGORY_TAOIST_HEAL,
	SKILL_MEDIUM_CATEGORY_TAOIST_BUF,

	SKILL_MEDIUM_CATEGORY_THIEF_ATTACK,
	SKILL_MEDIUM_CATEGORY_THIEF_HEAL,
	SKILL_MEDIUM_CATEGORY_THIEF_BUF,

	SKILL_MEDIUM_CATEGORY_ARCHER_ATTACK,
	SKILL_MEDIUM_CATEGORY_ARCHER_HEAL,
	SKILL_MEDIUM_CATEGORY_ARCHER_BUF
};

enum class en_SkillType : int16
{
	SKILL_TYPE_NONE = 0,
	SKILL_DEFAULT_ATTACK = 1,

	SKILL_KNIGHT_FIERCE_ATTACK,
	SKILL_KNIGHT_CONVERSION_ATTACK,
	SKILL_KNIGHT_SMASH_WAVE,
	SKILL_KNIGHT_SHAEHONE,
	SKILL_KNIGHT_CHOHONE,
	SKILL_KNIGHT_CHARGE_POSE,

	SKILL_SHAMAN_FLAME_HARPOON,
	SKILL_SHAMAN_ROOT,
	SKILL_SHAMAN_ICE_CHAIN,
	SKILL_SHAMAN_ICE_WAVE,
	SKILL_SHAMAN_LIGHTNING_STRIKE,
	SKILL_SHAMAN_HELL_FIRE,

	SKILL_TAIOIST_DIVINE_STRIKE,
	SKILL_TAIOIST_HEALING_LIGHT,
	SKILL_TAIOIST_HEALING_WIND,
	SKILL_TAIOIST_ROOT,

	SKILL_THIEF_QUICK_CUT,

	SKILL_ARCHER_SNIFING,

	SKILL_SHOCK_RELEASE,

	SKILL_SLIME_NORMAL = 3000,
	SKILL_BEAR_NORMAL
};

enum class en_QuickSlotBar : int8
{
	QUICK_SLOT_BAR_SIZE = 2,
	QUICK_SLOT_BAR_SLOT_SIZE = 5
};

enum class en_TileMapEnvironment : int8
{
	TILE_MAP_NONE = 0,
	TILE_MAP_WALL,
	TILE_MAP_TREE,
	TILE_MAP_STONE,
	TILE_MAP_SLIME,
	TILE_MAP_BEAR
};

enum class en_MapItemInfo : int8
{
	MAP_ITEM_COUNT_MAX = 20
};

struct st_PositionInfo
{
	en_CreatureState State;
	int32 CollisionPositionX;
	int32 CollisionPositionY;
	en_MoveDir MoveDir;
	float PositionX;
	float PositionY;
};

struct st_StatInfo
{
	int32 Level;
	int32 HP;
	int32 MaxHP;
	int32 MP;
	int32 MaxMP;
	int32 DP;
	int32 MaxDP;
	int16 AutoRecoveryHPPercent;
	int16 AutoRecoveryMPPercent;
	int32 MinMeleeAttackDamage;
	int32 MaxMeleeAttackDamage;
	int16 MeleeAttackHitRate;
	int16 MagicDamage;
	int16 MagicHitRate;
	int32 Defence;
	int16 EvasionRate;
	int16 MeleeCriticalPoint;
	int16 MagicCriticalPoint;
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

struct st_SkillInfo
{
	bool IsQuickSlotUse;	 // �����Կ� ��ϵǾ� �ִ��� ����
	en_SkillLargeCategory SkillLargeCategory; // ��ų ��з�
	en_SkillMediumCategory SkillMediumCategory; // ��ų �ߺз�
	en_SkillType SkillType;	 // ��ų ����
	int8 SkillLevel;		 // ��ų ����
	wstring SkillName;		 // ��ų �̸�
	int32 SkillCoolTime;	 // ��ų ��Ÿ��
	int32 SkillCastingTime;  // ��ų ĳ���� Ÿ��
	float SkillTargetEffectTime;
	wstring SkillImagePath;	 // ��ų �̹��� ���
	bool CanSkillUse;		 // ��ų�� ��� �� �� �ִ��� ����	

	st_SkillInfo()
	{
		IsQuickSlotUse = false;
		SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_NONE;
		SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_NONE;
		SkillType = en_SkillType::SKILL_TYPE_NONE;
		SkillLevel = 0;
		SkillName = L"";
		SkillCoolTime = 0;
		SkillCastingTime = 0;
		SkillTargetEffectTime = 0;
		SkillImagePath = L"";
		CanSkillUse = true;
	}
};

struct st_QuickSlotBarSlotInfo
{
	int64 AccountDBId; // ������ ���� ������ Account
	int64 PlayerDBId;  // ������ ���� ������ Player	
	int8 QuickSlotBarIndex; // ������ Index
	int8 QuickSlotBarSlotIndex; // ������ ���� Index
	int16 QuickSlotKey;   // �����Կ� ������ Ű��
	st_SkillInfo QuickBarSkillInfo;	// �����Կ� ����� ��ų ����
	bool CanQuickSlotUse = true; // �������� ����� �� �ִ��� ������
};