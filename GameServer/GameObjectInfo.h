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
	OBJECT_ITEM_SLIME_GEL,
	OBJECT_ITEM_LEATHER,
	OBJECT_ITEM_BRONZE_COIN,
	OBJECT_ITEM_SLIVER_COIN,
	OBJECT_ITEM_GOLD_COIN,
	OBJECT_ITEM_SKILL_BOOK,
	OBJECT_ITEM_WOOD_LOG,
	OBJECT_ITEM_STONE,	
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
	PATROL,
	MOVING,
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

enum class en_ItemType : int16
{
	ITEM_TYPE_NONE = 0,

	ITEM_TYPE_WEAPON_WOOD_SWORD = 1,

	ITEM_TYPE_ARMOR_WOOD_ARMOR = 100,
	ITEM_TYPE_ARMOR_LETHER_HAT = 101,

	ITEM_TYPE_POTION_HEAL_SMALL = 200,
	ITEM_TYPE_SKILL_BOOK_CHOHONE = 300,

	ITEM_TYPE_LEATHER = 2000,
	ITEM_TYPE_SLIMEGEL = 2001,
	ITEM_TYPE_BRONZE_COIN = 2002,
	ITEM_TYPE_SLIVER_COIN = 2003,
	ITEM_TYPE_GOLD_COIN = 2004,
	ITEM_TYPE_STONE = 2005,
	ITEM_TYPE_WOOD_LOG = 2006,
	ITEM_TYPE_WOOD_FLANK = 2007
};

enum class en_ItemCategory : int8
{
	ITEM_CATEGORY_NONE = 0,
	ITEM_CATEGORY_WEAPON,
	ITEM_CATEGORY_ARMOR,
	ITEM_CATEGORY_FOOD,
	ITEM_CATEGORY_POTION,
	ITEM_CATEGORY_SKILLBOOK,
	ITEM_CATEGORY_MATERIAL	
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
	EFFECT_HELAING_MYSELF	
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
	TILE_MAP_STONE
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
	int64 ItemDBId;				// ������ DB�� ����Ǿ� �ִ� ID		
	bool IsQuickSlotUse;        // �����Կ� ��ϵǾ� �ִ��� ���� 
	en_ItemCategory ItemCategory; // ������ ����
	en_ItemType ItemType;		// ������ Ÿ��
	wstring ItemName;			// ������ �̸�
	int16 ItemCount;			// ����
	wstring ThumbnailImagePath; // �̹��� ���
	bool IsEquipped;			// �������� ������ �� �ִ���	
	int8 SlotIndex;				// ���� ��ȣ

	st_ItemInfo()
	{
		ItemDBId = 0;
		IsQuickSlotUse = false;
		ItemCategory = en_ItemCategory::ITEM_CATEGORY_NONE;
		ItemType = en_ItemType::ITEM_TYPE_NONE;
		ItemName = L"";
		ItemCount = 0;
		ThumbnailImagePath = L"";
		IsEquipped = false;
		SlotIndex  = -1;
	}
};

struct st_SkillInfo
{
	bool IsQuickSlotUse;    // �����Կ� ��ϵǾ� �ִ��� ����
	en_SkillType SkillType; // ��ų ����
	int8 SkillLevel = 0;		 // ��ų ����
	wstring SkillName;		 // ��ų �̸�
	int32 SkillCoolTime = 0;	 // ��ų ��Ÿ��	
	wstring SkillImagePath; // ��ų �̹��� ���
	bool CanSkillUse = true; // ��ų�� ��� �� �� �ִ��� ����	
};

struct st_QuickSlotBarSlotInfo
{
	int64 AccountDBId; // ������ ���� ������ Account
	int64 PlayerDBId;  // ������ ���� ������ Player	
	int8 QuickSlotBarIndex; // ������ Index
	int8 QuickSlotBarSlotIndex; // ������ ���� Index
	wstring QuickSlotKey;   // �����Կ� ������ Ű��
	st_SkillInfo QuickBarSkillInfo;	// �����Կ� ����� ��ų ����
	bool CanQuickSlotUse = true; // �������� ����� �� �ִ��� ������
};

struct st_CraftingMaterialItemInfo
{
	int64 AccountDBId; // ����� ������ �ִ� Account
	int64 PlayerDBId; // ����� ������ �ִ� Player
	en_ItemType MaterialItemType; // ����� ����
	wstring MaterialItemName; // ����� �̸�
	int16 ItemCount; // ����� �ʿ� ����
	wstring MaterialItemImagePath; // ����� �̹��� ���
};

struct st_CraftingCompleteItem
{
	en_ItemType CompleteItemType; // �ϼ� ������ ����
	wstring CompleteItemName; // �ϼ� ������ �̸�
	wstring CompleteItemImagePath; // �ϼ� ������ �̹��� ���
	vector<st_CraftingMaterialItemInfo> Materials; // ������ ���鶧 �ʿ��� ����
};

struct st_CraftingItemCategory
{
	en_ItemCategory CategoryType; // ������ ����
	wstring CategoryName; // ������ ���� �̸�
	vector<st_CraftingCompleteItem> CompleteItems; // ���ֿ� ���� �ϼ� �����۵�
};