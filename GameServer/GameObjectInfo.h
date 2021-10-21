#pragma once

enum class en_GameObjectType : int16
{
	NORMAL,
		
	OBJECT_PLAYER,
	OBJECT_MELEE_PLAYER,
	OBJECT_MAGIC_PLAYER,
	
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
	OBJECT_ITEM_CONSUMABLE_SKILL_BOOK_CHOHONE,
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
	OBJECT_ITEM_MATERIAL_YARN
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
	SPAWN_IDLE,
	IDLE,
	PATROL,
	MOVING,
	RETURN_SPAWN_POSITION,
	ATTACK,
	SPELL,
	DEAD,	
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

enum class en_QuickSlotBar : int8
{
	QUICK_SLOT_BAR_SIZE = 2,
	QUICK_SLOT_BAR_SLOT_SIZE = 5
};

enum class en_LargeItemCategory : int8
{
	ITEM_LARGE_CATEGORY_NONE = 0,
	ITEM_LARGE_CATEGORY_WEAPON,
	ITEM_LARGE_CATEGORY_ARMOR,
	ITEM_LARGE_CATEGORY_FOOD,
	ITEM_LARGE_CATEGORY_POTION,
	ITEM_LARGE_CATEGORY_SKILLBOOK,
	ITEM_LARGE_CATEGORY_MATERIAL
};

enum class en_MediumItemCategory : int8
{
	ITEM_MEDIUM_CATEGORY_NONE = 0,
	ITEM_MEDIUM_CATEGORY_SWORD,
	ITEM_MEDIUM_CATEGORY_HAT,
	ITEM_MEDIUM_CATEGORY_WEAR,
	ITEM_MEDIUM_CATEGORY_GLOVE,
	ITEM_MEDIUM_CATEGORY_BOOT,
	ITEM_MEDIUM_CATEGORY_HEAL,	
	ITEM_MEDIUM_CATEGORY_MANA	
};

enum class en_SmallItemCategory : int16
{
	ITEM_SMALL_CATEGORY_NONE = 0,

	ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD = 1,

	ITEM_SMALL_CATEGORY_ARMOR_WEAR_WOOD = 100,
	ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER = 101,
	ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER = 102,

	ITEM_SMALL_CATEGORY_POTION_HEAL_SMALL = 200,
	ITEM_SMALL_CATEGORY_SKILLBOOK_CHOHONE = 300,

	ITEM_SMALL_CATEGORY_MATERIAL_LEATHER = 2000,
	ITEM_SMALL_CATEGORY_MATERIAL_SLIMEGEL = 2001,
	ITEM_SMALL_CATEGORY_MATERIAL_BRONZE_COIN = 2002,
	ITEM_SMALL_CATEGORY_MATERIAL_SLIVER_COIN = 2003,
	ITEM_SMALL_CATEGORY_MATERIAL_GOLD_COIN = 2004,
	ITEM_SMALL_CATEGORY_MATERIAL_STONE = 2005,
	ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG = 2006,
	ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK = 2007,
	ITEM_SMALL_CATEGORY_MATERIAL_YARN = 2008
};

enum class en_SkillType : int16
{
	SKILL_TYPE_NONE = 0,
	SKILL_NORMAL,
	SKILL_KNIGHT_FIERCE_ATTACK,
	SKILL_KNIGHT_CONVERSION_ATTACK,
	SKILL_KNIGHT_CHARGE_POSE,
	SKILL_KNIGHT_SHAEHONE,
	SKILL_KNIGHT_CHOHONE,	
	SKILL_KNIGHT_SMASH_WAVE,	
	SKILL_SHAMNA_FLAME_HARPOON,
	SKILL_SHAMAN_HEALING_LIGHT,
	SKILL_SHAMAN_HEALING_WIND,	
	SKILL_SLIME_NORMAL,
	SKILL_BEAR_NORMAL
};

enum class en_EffectType : int16
{
	EFFECT_TYPE_NONE = 0,
	EFFECT_NORMAL_ATTACK_TARGET_HIT,	
	EFFECT_SMASH_WAVE,
	EFFECT_CHOHONE_TARGET_HIT,
	EFFECT_SHAHONE_TARGET_HIT,
	EFFECT_CHARGE_POSE,
	EFFECT_FLAME_HARPOON_TARGET,
	EFFECT_HEALING_LIGHT_TARGET,
	EFFECT_HEALING_WIND_TARGET,
	EFFECT_HELAING_MYSELF,
	EFFECT_OBJECT_SPAWN
};

enum class en_ErrorType : int16
{
	ERROR_SKILL_COOLTIME,
	ERROR_NON_SELECT_OBJECT,
	ERROR_DISTANCE
};

enum class en_ConsumableType : int16
{
	NONE,
	POTION,
	SKILL_BOOK
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
	int32 MP;
	int32 MaxMP;
	int32 DP;
	int32 MaxDP;
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
	bool IsQuickSlotUse;        // 퀵슬롯에 등록되어 있는지 여부 
	en_LargeItemCategory ItemLargeCategory; // 아이템 대분류
	en_MediumItemCategory ItemMediumCategory; // 아이템 중분류
	en_SmallItemCategory ItemSmallCategory;		// 아이템 소분류
	wstring ItemName;			// 아이템 이름
	int16 ItemCount;			// 개수
	wstring ThumbnailImagePath; // 이미지 경로
	bool IsEquipped;			// 아이템을 착용할 수 있는지	
	int8 SlotIndex;				// 슬롯 번호

	st_ItemInfo()
	{
		ItemDBId = 0;
		IsQuickSlotUse = false;
		ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_NONE;
		ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_NONE;
		ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_NONE;
		ItemName = L"";
		ItemCount = 0;
		ThumbnailImagePath = L"";
		IsEquipped = false;
		SlotIndex  = -1;
	}

	void InventoryItemInit()
	{
		ItemDBId = 0;
		IsQuickSlotUse = false;
		ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_NONE;
		ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_NONE;
		ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_NONE;
		ItemName = L"";
		ItemCount = 0;
		ThumbnailImagePath = L"";
		IsEquipped = false;
	}
};

struct st_SkillInfo
{
	bool IsQuickSlotUse;    // 퀵슬롯에 등록되어 있는지 여부
	en_SkillType SkillType; // 스킬 종류
	int8 SkillLevel = 0;		 // 스킬 레벨
	wstring SkillName;		 // 스킬 이름
	int32 SkillCoolTime = 0;	 // 스킬 쿨타임
	int32 SkillCastingTime = 0;  // 스킬 캐스팅 타임
	wstring SkillImagePath; // 스킬 이미지 경로
	bool CanSkillUse = true; // 스킬을 사용 할 수 있는지 여부	
};

struct st_QuickSlotBarSlotInfo
{
	int64 AccountDBId; // 퀵슬롯 슬롯 소유한 Account
	int64 PlayerDBId;  // 퀵슬롯 슬롯 소유한 Player	
	int8 QuickSlotBarIndex; // 퀵슬롯 Index
	int8 QuickSlotBarSlotIndex; // 퀵슬롯 슬롯 Index
	wstring QuickSlotKey;   // 퀵슬롯에 연동된 키값
	st_SkillInfo QuickBarSkillInfo;	// 퀵슬롯에 등록할 스킬 정보
	bool CanQuickSlotUse = true; // 퀵슬롯을 사용할 수 있는지 없는지
};

struct st_CraftingMaterialItemInfo
{
	int64 AccountDBId; // 재료템 가지고 있는 Account
	int64 PlayerDBId; // 재료템 가지고 있는 Player
	en_SmallItemCategory MaterialItemType; // 재료템 종류
	wstring MaterialItemName; // 재료템 이름
	int16 ItemCount; // 재료템 필요 개수
	wstring MaterialItemImagePath; // 재료템 이미지 경로
};

struct st_CraftingCompleteItem
{
	en_SmallItemCategory CompleteItemType; // 완성 제작템 종류
	wstring CompleteItemName; // 완성 제작템 이름
	wstring CompleteItemImagePath; // 완성 제작템 이미지 경로
	vector<st_CraftingMaterialItemInfo> Materials; // 제작템 만들때 필요한 재료들
};

struct st_CraftingItemCategory
{
	en_LargeItemCategory CategoryType; // 제작템 범주
	wstring CategoryName; // 제작템 범주 이름
	vector<st_CraftingCompleteItem> CompleteItems; // 범주에 속한 완성 제작템들
};