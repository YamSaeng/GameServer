#pragma once
#include "Type.h"

enum class en_CreatureState : int8
{
	SPAWN_READY,
	SPAWN_IDLE,
	IDLE,
	PATROL,
	MOVING,
	STOP,
	RETURN_SPAWN_POSITION,
	ATTACK,
	SPELL,
	GATHERING,
	READY_DEAD,
	DEAD
};

enum class en_MoveDir : int8
{
	UP,
	DOWN,
	LEFT,
	RIGHT,
	LEFT_UP,
	LEFT_DOWN,
	RIGHT_UP,
	RIGHT_DOWN
};

enum class en_GameObjectType : int16
{
	OBJECT_NON_TYPE,

	OBJECT_PLAYER,
	OBJECT_NON_PLAYER,

	OBJECT_MONSTER,
	OBJECT_SLIME,
	OBJECT_BEAR,

	OBJECT_ENVIRONMENT,
	OBJECT_STONE,
	OBJECT_TREE,
	OBJECT_WALL,

	OBJECT_ARCHITECTURE,
	OBJECT_ARCHITECTURE_CRAFTING_TABLE,
	OBJECT_ARCHITECTURE_CRAFTING_DEFAULT_CRAFTING_TABLE,
	OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE,
	OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL,

	OBJECT_CROP,
	OBJECT_CROP_POTATO,
	OBJECT_CROP_CORN,

	OBJECT_STORAGE,
	OBJECT_STORAGE_BOX,

	OBJECT_ITEM,
	OBJECT_ITEM_WEAPON,
	OBJECT_ITEM_WEAPON_WOOD_SWORD,
	OBJECT_ITEM_WEAPON_WOOD_SHIELD,

	OBJECT_ITEM_TOOL,
	OBJECT_ITEM_TOOL_FARMING_SHOVEL,

	OBJECT_ITEM_ARMOR,
	OBJECT_ITEM_ARMOR_LEATHER_ARMOR,

	OBJECT_ITEM_ARMOR_LEATHER_HELMET,
	OBJECT_ITEM_ARMOR_LEATHER_BOOT,

	OBJECT_ITEM_CONSUMABLE,
	OBJECT_ITEM_CONSUMABLE_SKILL_BOOK,
	OBJECT_ITEM_CONSUMABLE_HEALTH_RESTORATION_POTION_SMALL,
	OBJECT_ITEM_CONSUMABLE_MANA_RESTORATION_POTION_SMALL,

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
	OBJECT_ITEM_MATERIAL_CHAR_COAL,
	OBJECT_ITEM_MATERIAL_COPPER_NUGGET,
	OBJECT_ITEM_MATERIAL_COPPER_INGOT,
	OBJECT_ITEM_MATERIAL_IRON_NUGGET,
	OBJECT_ITEM_MATERIAL_IRON_INGOT,

	OBJECT_ITEM_CROP_SEED,
	OBJECT_ITEM_CROP_SEED_POTATO,
	OBJECT_ITEM_CROP_SEED_CORN,

	OBJECT_ITEM_CROP_FRUIT,
	OBJECT_ITEM_CROP_FRUIT_POTATO,
	OBJECT_ITEM_CROP_FRUIT_CORN,

	OBJECT_ITEM_STORAGE,
	OBJECT_ITEM_STORAGE_BOX,

	OBJECT_PLAYER_DUMMY = 32000
};

enum class en_SkillLargeCategory : int8
{
	SKILL_LARGE_CATEGORY_NONE = 0,
	SKILL_LARGE_CATEGORY_PUBLIC,

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

	SKILL_MEDIUM_CATEGORY_PUBLIC_ACTIVE_ATTACK,
	SKILL_MEDIUM_CATEGORY_PUBLIC_ACTIVE_BUF,
	SKILL_MEDIUM_CATEGORY_PUBLIC_PASSIVE,

	SKILL_MEDIUM_CATEGORY_FIGHT_ACTIVE_ATTACK,
	SKILL_MEDIUM_CATEGORY_FIGHT_ACTIVE_BUF,
	SKILL_MEDIUM_CATEGORY_FIGHT_PASSIVE,

	SKILL_MEDIUM_CATEGORY_PROTECTION_ACTIVE_ATTACK,
	SKILL_MEDIUM_CATEGORY_PROTECTION_ACTIVE_BUF,
	SKILL_MEDIUM_CATEOGRY_PROTECTION_PASSIVE,

	SKILL_MEDIUM_CATEGORY_ASSASSINATION_ACTIVE_ATTACK,
	SKILL_MEDIUM_CATEGORY_ASSASSINATION_ACTIVE_BUF,
	SKILL_MEDIUM_CATEGORY_ASSASSINATION_PASSIVE,

	SKILL_MEDIUM_CATEGORY_SPELL_ACTIVE_ATTACK,
	SKILL_MEDIUM_CATEGORY_SPELL_ACTIVE_BUF,
	SKILL_MEDIUM_CATEGORY_SPELL_PASSIVE,

	SKILL_MEDIUM_CATEGORY_SHOOTING_ACTIVE_ATTACK,
	SKILL_MEDIUM_CATEGORY_SHOOTING_ACTIVE_BUF,
	SKILL_MEDIUM_CATEGORY_SHOOTING_PASSIVE,

	SKILL_MEDIUM_CATEGORY_DISCIPLINE_ACTIVE_ATTACK,
	SKILL_MEDIUM_CATEGORY_DISCIPLINE_ACTIVE_HEAL,
	SKILL_MEDIUM_CATEGORY_DISCIPLINE_ACTIVE_BUF,
	SKILL_MEDIUM_CATEGORY_DISCIPLINE_PASSIVE
};

enum class en_SkillType : int16
{
	SKILL_TYPE_NONE = 0,
	SKILL_DEFAULT_ATTACK = 1,

	SKILL_FIGHT_TWO_HAND_SWORD_MASTER,

	SKILL_FIGHT_ACTIVE_ATTACK_FIERCE_ATTACK,
	SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK,
	SKILL_FIGHT_ACTIVE_ATTACK_SMASH_WAVE,
	SKILL_FIGHT_ACTIVE_ATTACK_SHAHONE,
	SKILL_FIGHT_ACTIVE_ATTACK_CHOHONE,
	SKILL_FIGHT_ACTIVE_BUF_CHARGE_POSE,

	SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_SMASH,

	SKILL_SPELL_ACTIVE_ATTACK_FLAME_HARPOON,
	SKILL_SPELL_ACTIVE_ATTACK_ROOT,
	SKILL_SPELL_ACTIVE_ATTACK_ICE_CHAIN,
	SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE,
	SKILL_SPELL_ACTIVE_ATTACK_LIGHTNING_STRIKE,
	SKILL_SPELL_ACTIVE_ATTACK_HEL_FIRE,
	SKILL_SPELL_ACTIVE_BUF_TELEPORT,

	SKILL_DISCIPLINE_ACTIVE_ATTACK_DIVINE_STRIKE,
	SKILL_DISCIPLINE_ACTIVE_ATTACK_ROOT,
	SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_LIGHT,
	SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_WIND,

	SKILL_ASSASSINATION_ACTIVE_ATTACK_QUICK_CUT,
	SKILL_ASSASSINATION_ACTIVE_ATTACK_FAST_CUT,
	SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_ATTACK,
	SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_STEP,
	SKILL_ASSASSINATION_ACTIVE_BUF_WEAPON_POISON,

	SKILL_SHOOTING_ACTIVE_ATTACK_SNIFING,

	SKILL_PUBLIC_ACTIVE_BUF_SHOCK_RELEASE,

	SKILL_SLIME_NORMAL = 3000,
	SKILL_BEAR_NORMAL
};

enum class en_QuickSlotBar : int8
{
	QUICK_SLOT_BAR_SIZE = 2,
	QUICK_SLOT_BAR_SLOT_SIZE = 5
};

enum class en_MapObjectInfo : int8
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
	en_MoveDir MoveDir;
	int32 CollisionPositionX;
	int32 CollisionPositionY;	
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
	float MagicHitRate;
	int32 Defence;
	int16 EvasionRate;
	int16 MeleeCriticalPoint;
	int16 MagicCriticalPoint;
	int16 StatusAbnormalResistance;
	float Speed;
	float MaxSpeed;
};

struct st_GameObjectInfo
{
	int64 ObjectId;
	wstring ObjectName;
	int8 ObjectCropStep;
	int8 ObjectCropMaxStep;
	int8 ObjectSkillMaxPoint;
	int8 ObjectSkillPoint;	
	st_PositionInfo ObjectPositionInfo;
	st_StatInfo ObjectStatInfo;
	en_GameObjectType ObjectType;
	int64 OwnerObjectId;
	en_GameObjectType OwnerObjectType;
	int16 ObjectWidth;
	int16 ObjectHeight;
	int8 PlayerSlotIndex;
};

struct st_SkillInfo
{
	bool IsQuickSlotUse;	 // 퀵슬롯에 등록되어 있는지 여부
	en_SkillLargeCategory SkillLargeCategory; // 스킬 대분류
	en_SkillMediumCategory SkillMediumCategory; // 스킬 중분류
	en_SkillType SkillType;	 // 스킬 종류
	int8 SkillLevel;		 // 스킬 레벨
	wstring SkillName;		 // 스킬 이름
	int32 SkillCoolTime;	 // 스킬 쿨타임
	int32 SkillCastingTime;  // 스킬 캐스팅 타임
	int64 SkillDurationTime; // 스킬 지속 시간
	int64 SkillDotTime;      // 스킬 도트 시간 
	int64 SkillRemainTime;   // 스킬 남은 시간
	float SkillTargetEffectTime;
	wstring SkillImagePath;	 // 스킬 이미지 경로
	bool CanSkillUse;		 // 스킬을 사용 할 수 있는지 여부	

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
		SkillDurationTime = 0;
		SkillDotTime = 0;
		SkillRemainTime = 0;
		SkillTargetEffectTime = 0;
		SkillImagePath = L"";
		CanSkillUse = true;
	}
};

struct st_QuickSlotBarSlotInfo
{
	int64 AccountDBId; // 퀵슬롯 슬롯 소유한 Account
	int64 PlayerDBId;  // 퀵슬롯 슬롯 소유한 Player	
	int8 QuickSlotBarIndex; // 퀵슬롯 Index
	int8 QuickSlotBarSlotIndex; // 퀵슬롯 슬롯 Index
	int16 QuickSlotKey;   // 퀵슬롯에 연동된 키값
	st_SkillInfo QuickBarSkillInfo;	// 퀵슬롯에 등록할 스킬 정보
	bool CanQuickSlotUse = true; // 퀵슬롯을 사용할 수 있는지 없는지
};