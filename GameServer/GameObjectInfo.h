#pragma once

#include "Vector2.h"

class CGameObject;
class CGameServerMessage;
class CSkill;
class CItem;

enum class en_GameObjectStatusType : int64
{
	STATUS_ABNORMAL_NONE =								0b00000000000000000000000000000000,	
	STATUS_ABNORMAL_FIGHT_WRATH_ATTACK =				0b00000000000000000000000000000001,
	STATUS_ABNORMAL_FIGHT_JUMPING_ATTACK =				0b00000000000000000000000000000010,
	STATUS_ABNORMAL_FIGHT_FIERCING_WAVE =				0b00000000000000000000000000000100,
	STATUS_ABNORMAL_PROTECTION_LAST_ATTACK =			0b00000000000000000000000000001000,
	STATUS_ABNORMAL_PROTECTION_SHIELD_SMASH =			0b00000000000000000000000000010000,
	STATUS_ABNORMAL_PROTECTION_SHIELD_COUNTER =			0b00000000000000000000000000100000,
	STATUS_ABNORMAL_PROTECTION_SWORD_STORM =			0b00000000000000000000000001000000,
	STATUS_ABNORMAL_PROTECTION_CAPTURE =				0b00000000000000000000000010000000,
	STATUS_ABNORMAL_SPELL_ICE_CHAIN =					0b00000000000000000000000100000000,
	STATUS_ABNORMAL_SPELL_ICE_WAVE =					0b00000000000000000000001000000000,
	STATUS_ABNORMAL_SPELL_ROOT =						0b00000000000000000000010000000000,
	STATUS_ABNORMAL_SPELL_SLEEP =						0b00000000000000000000100000000000,
	STATUS_ABNORMAL_SPELL_WINTER_BINDING =				0b00000000000000000001000000000000,
	STATUS_ABNORMAL_SPELL_LIGHTNING_STRIKE =			0b00000000000000000010000000000000,
	STATUS_ABNORMAL_DISCIPLINE_ROOT =					0b00000000000000000100000000000000,
	STATUS_ABNORMAL_DISCIPLINE_JUDGMENT =				0b00000000000000001000000000000000,
	STATUS_ABNORMAL_ASSASSINATION_POISON_INJECTION =	0b00000000000000010000000000000000,
	STATUS_ABNORMAL_ASSASSINATION_POISON_STUN =			0b00000000000000100000000000000000,
	STATUS_ABNORMAL_ASSASSINATION_BACK_STEP =			0b00000000000001000000000000000000,
	STATUS_ABNORMAL_SHOOTING_NOOSE_ARROW =			    0b00000000000010000000000000000000,
	STATUS_ABNORMAL_SHOOTING_ASSAULT_ARROW =			0b00000000000100000000000000000000,
	STATUS_ABNORMAL_SHOOTING_IMPULSE_ARROW =			0b00000000001000000000000000000000,

	STATUS_ABNORMAL_FIGHT_WRATH_ATTACK_MASK =				0b11111111111111111111111111111110,
	STATUS_ABNORMAL_FIGHT_JUMPING_ATTACK_MASK =				0b11111111111111111111111111111101,
	STATUS_ABNORMAL_FIGHT_FIERCING_WAVE_MASK =				0b11111111111111111111111111111011,
	STATUS_ABNORMAL_PROTECTION_LAST_MASK =					0b11111111111111111111111111110111,
	STATUS_ABNORMAL_PROTECTION_SHIELD_SMASH_MASK =			0b11111111111111111111111111101111,
	STATUS_ABNORMAL_PROTECTION_SHIELD_COUNTER_MASK =		0b11111111111111111111111111011111,
	STATUS_ABNORMAL_PROTECTION_SWORD_STORM_MASK =			0b11111111111111111111111110111111,
	STATUS_ABNORMAL_PROTECTION_CAPTURE_MASK =				0b11111111111111111111111101111111,			
	STATUS_ABNORMAL_SPELL_ICE_CHAIN_MASK =					0b11111111111111111111111011111111,
	STATUS_ABNORMAL_SPELL_ICE_WAVE_MASK =					0b11111111111111111111110111111111,
	STATUS_ABNORMAL_SPELL_ROOT_MASK =						0b11111111111111111111101111111111,				
	STATUS_ABNORMAL_SPELL_SLEEP_MASK =						0b11111111111111111111011111111111,				
	STATUS_ABNORMAL_SPELL_WINTER_BINDING_MASK =				0b11111111111111111110111111111111,				
	STATUS_ABNORMAL_SPELL_LIGHTNING_STRIKE_MASK =			0b11111111111111111101111111111111,			
	STATUS_ABNORMAL_DISCIPLINE_ROOT_MASK =					0b11111111111111111011111111111111,			
	STATUS_ABNORMAL_DISCIPLINE_JUDGMENT_MASK =				0b11111111111111110111111111111111,			
	STATUS_ABNORMAL_ASSASSINATION_POISON_INJECTION_MASK =	0b11111111111111101111111111111111,				
	STATUS_ABNORMAL_ASSASSINATION_POISON_STUN_MASK =		0b11111111111111011111111111111111,				
	STATUS_ABNORMAL_ASSASSINATION_BACK_STEP_MASK =			0b11111111111101111111111111111111,				
	STATUS_ABNORMAL_ASSASSINATION_NOOSE_ARROW_MASK =		0b11111111111011111111111111111111,
	STATUS_ABNORMAL_ASSASSINATION_ASSAULT_ARROW_MASK =		0b11111111110111111111111111111111,
	STATUS_ABNORMAL_ASSASSINATION_IMPULSE_ARROW_MASK =		0b11111111101111111111111111111111,
};

enum class en_GameObjectType : int16
{
	OBJECT_NON_TYPE,

	OBJECT_PLAYER,	
	OBJECT_NON_PLAYER_GENERAL_MERCHANT,

	OBJECT_MONSTER,
	OBJECT_GOBLIN,	

	OBJECT_ENVIRONMENT,
	OBJECT_STONE,
	OBJECT_TREE,

	OBJECT_BUILDING,
	OBJECT_BUILDING_GOVERNMENT_OFFICE,

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
	OBJECT_STORAGE_SMALL_BOX,

	OBJECT_SKILL,
	OBJECT_SKILL_SWORD_BLADE,
	OBJECT_SKILL_FLAME_BOLT,
	OBJECT_SKILL_DIVINE_BOLT,

	OBJECT_ITEM,
	OBJECT_ITEM_WEAPON,
	OBJECT_ITEM_WEAPON_WOOD_DAGGER,
	OBJECT_ITEM_WEAPON_WOOD_LONG_SWORD,
	OBJECT_ITEM_WEAPON_WOOD_GREAT_SWORD,
	OBJECT_ITEM_WEAPON_WOOD_SHIELD,
	OBJECT_ITEM_WEAPON_WOOD_BOW,
	
	OBJECT_ITEM_TOOL,
	OBJECT_ITEM_TOOL_FARMING_SHOVEL,

	OBJECT_ITEM_ARMOR,
	OBJECT_ITEM_ARMOR_LEATHER_ARMOR,

	OBJECT_ITEM_ARMOR_LEATHER_HELMET,
	OBJECT_ITEM_ARMOR_LEATHER_BOOT,

	OBJECT_ITEM_CONSUMABLE,	
	OBJECT_ITEM_CONSUMABLE_HEALTH_RESTORATION_POTION_SMALL,
	OBJECT_ITEM_CONSUMABLE_MANA_RESTORATION_POTION_SMALL,

	OBJECT_ITEM_MATERIAL,	
	OBJECT_ITEM_MATERIAL_LEATHER,
	OBJECT_ITEM_MATERIAL_COIN,	
	OBJECT_ITEM_MATERIAL_WOOD_LOG,
	OBJECT_ITEM_MATERIAL_STONE,
	OBJECT_ITEM_MATERIAL_WOOD_FLANK,
	OBJECT_ITEM_MATERIAL_YARN,
	OBJECT_ITEM_MATERIAL_FABRIC,
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

	OBJECT_RECT_COLLISION,

	OBJECT_PLAYER_DUMMY = 32000
};

enum class en_NonPlayerType : int8
{
	NON_PLAYER_CHARACTER_NON_TYPE,

	NON_PLAYER_CHARACTER_일반_상인
};

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
	CRAFTING,	
	ROOTING,
	DEAD
};

enum class en_MonsterState : int8
{
	MONSTER_IDLE,
	MONSTER_READY_PATROL,
	MONSTER_PATROL,
	MONSTER_READY_MOVE,
	MONSTER_MOVE,
	MONSTER_READY_ATTACK,
	MONSTER_ATTACK
};

enum class en_ItemState : int8
{
	ITEM_IDLE,
	ITEM_READY_MOVE,
	ITEM_MOVE
};

enum class en_MessageType : int8
{
	MESSAGE_TYPE_CHATTING,
	MESSAGE_TYPE_DAMAGE_CHATTING,
	MESSAGE_TYPE_SYSTEM	
};

enum class en_StateChange : int16
{
	MOVE_TO_STOP,
	SPELL_TO_IDLE,
};

enum class en_ObjectNetworkState : int8
{
	OBJECT_NETWORK_STATE_READY,	
	OBJECT_NETWORK_STATE_LIVE,
	OBJECT_NETWORK_STATE_CHARACTER_SELECT,
	OBJECT_NETWORK_STATE_LEAVE
};

enum class en_InventoryManager : int8
{
	INVENTORY_DEFAULT_WIDH_SIZE = 10,
	INVENTORY_DEFAULT_HEIGHT_SIZE = 10
};

enum class en_QuickSlotBar : int8
{
	QUICK_SLOT_BAR_SIZE = 2,
	QUICK_SLOT_BAR_SLOT_SIZE = 5
};

enum class en_UIObjectInfo : int16
{
	UI_OBJECT_INFO_NONE = 0,

	UI_OBJECT_INFO_CRAFTING_TABLE_COMMON,
	UI_OBJECT_INFO_CRAFTING_TABLE_FURNACE,
	UI_OBJECT_INFO_CRAFTING_TABLE_SAWMILL
};

enum class en_LargeItemCategory : int8
{
	ITEM_LARGE_CATEGORY_NONE = 0,
	ITEM_LARGE_CATEGORY_ARCHITECTURE,
	ITEM_LARGE_CATEGORY_WEAPON,
	ITEM_LARGE_CATEGORY_ARMOR,
	ITEM_LARGE_CATEGORY_TOOL,
	ITEM_LARGE_CATEGORY_FOOD,
	ITEM_LARGE_CATEGORY_POTION,	
	ITEM_LARGE_CATEGORY_MATERIAL,
	ITEM_LARGE_CATEGORY_CROP
};

enum class en_MediumItemCategory : int8
{
	ITEM_MEDIUM_CATEGORY_NONE = 0,
	ITEM_MEDIUM_CATEGORY_CRAFTING_TABLE,
	ITEM_MEDIUM_CATEGORY_WEAPON_DAGGER,
	ITEM_MEDIUM_CATEGORY_WEAPON_LONG_SWORD,
	ITEM_MEDIUM_CATEGORY_WEAPON_GREAT_SWORD,	
	ITEM_MEDIUM_CATEGORY_WEAPON_SHIELD,
	ITEM_MEDIUM_CATEGORY_WEAPON_BOW,		
	ITEM_MEDIUM_CATEGORY_ARMOR_HAT,
	ITEM_MEDIUM_CATEGORY_ARMOR_WEAR,
	ITEM_MEDIUM_CATEGORY_ARMOR_GLOVE,
	ITEM_MEDIUM_CATEGORY_ARMOR_BOOT,
	ITEM_MEDIUM_CATEOGRY_FARMING,
	ITEM_MEDIUM_CATEGORY_HEAL,
	ITEM_MEDIUM_CATEGORY_MANA,
	ITEM_MEDIUM_CATEGORY_CROP_SEED,
	ITEM_MEDIUM_CATEGORY_CROP_FRUIT
};

enum class en_SmallItemCategory : int16
{
	ITEM_SMALL_CATEGORY_NONE = 0,

	ITEM_SMALL_CATEGORY_WEAPON_DAGGER_WOOD = 1,
	ITEM_SMALL_CATEGORY_WEAPON_LONG_SWORD_WOOD,	
	ITEM_SMALL_CATEGORY_WEAPON_GREAT_SWORD_WOOD,	
	ITEM_SAMLL_CATEGORY_WEAPON_SHIELD_WOOD,
	ITEM_SAMLL_CATEGORY_WEAPON_BOW_WOOD,

	ITEM_SMALL_CATEGORY_TOOL_FARMING_SHOVEL,

	ITEM_SMALL_CATEGORY_ARMOR_WEAR_LEATHER = 100,
	ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER,
	ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER,

	ITEM_SMALL_CATEGORY_POTION_HEALTH_RESTORATION_POTION_SMALL = 200,
	ITEM_SMALL_CATEGORY_POTION_MANA_RESTORATION_POTION_SMALL,	

	ITEM_SMALL_CATEGORY_MATERIAL_LEATHER = 2000,	
	ITEM_SMALL_CATEGORY_MATERIAL_COIN,	
	ITEM_SMALL_CATEGORY_MATERIAL_STONE,
	ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG,
	ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK,
	ITEM_SMALL_CATEGORY_MATERIAL_YARN,
	ITEM_SMALL_CATEGORY_MATERIAL_FABRIC,
	ITEM_SMALL_CATEGORY_MATERIAL_CHAR_COAL,
	ITEM_SMALL_CATEGORY_MATERIAL_COPPER_NUGGET,
	ITEM_SMALL_CATEGORY_MATERIAL_COPPER_INGOT,
	ITEM_SMALL_CATEGORY_MATERIAL_IRON_NUGGET,
	ITEM_SMALL_CATEGORY_MATERIAL_IRON_INGOT,

	ITEM_SMALL_CATEGORY_CROP_SEED_POTATO,
	ITEM_SMALL_CATEGORY_CROP_FRUIT_POTATO,
	ITEM_SMALL_CATEGORY_CROP_SEED_CORN,
	ITEM_SMALL_CATEGORY_CROP_FRUIT_CORN,

	ITEM_SMALL_CATEGORY_CRAFTING_DEFAULT_CRAFTING_TABLE = 5000,
	ITEM_SMALL_CATEGORY_CRAFTING_TABLE_FURANCE,
	ITEM_SMALL_CATEGORY_CRAFTING_TABLE_SAWMILL
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
	SKILL_GLOBAL_SKILL = 1,

	SKILL_DEFAULT_ATTACK,
	SKILL_PUBLIC_ACTIVE_BUF_SHOCK_RELEASE,

	SKILL_FIGHT_TWO_HAND_SWORD_MASTER,

	SKILL_FIGHT_ACTIVE_ATTACK_FIERCE_ATTACK,
	SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK,
	SKILL_FIGHT_ACTIVE_ATTACK_WRATH_ATTACK,
	SKILL_FIGHT_ACTIVE_ATTACK_SMASH_WAVE,
	SKILL_FIGHT_ACTIVE_ATTACK_FLY_KNIFE,
	SKILL_FIGHT_ACTIVE_ATTACK_COMBO_FLY_KNIFE,
	SKILL_FIGHT_ACTIVE_ATTACK_JUMPING_ATTACK,	
	SKILL_FIGHT_ACTIVE_ATTACK_PIERCING_WAVE,		
	SKILL_FIGHT_ACTIVE_BUF_CHARGE_POSE,
	SKILL_FIGHT_ACTIVE_BUF_COUNTER_ARMOR,

	SKILL_PROTECTION_ACTIVE_ATTACK_POWERFUL_ATTACK,
	SKILL_PROTECTION_ACTIVE_ATTACK_SHARP_ATTACK,
	SKILL_PROTECTION_ACTIVE_ATTACK_LAST_ATTACK,
	SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_SMASH,
	SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_COUNTER,
	SKILL_PROTECTION_ACTIVE_ATTACK_SWORD_STORM,
	SKILL_PROTECTION_ACTIVE_ATTACK_CAPTURE,
	SKILL_PROTECTION_ACTIVE_BUF_FURY,
	SKILL_PROTECTION_ACTIVE_DOUBLE_ARMOR,

	SKILL_SPELL_ACTIVE_ATTACK_FLAME_BOLT,
	SKILL_SPELL_ACTIVE_ATTACK_FLAME_BLAZE,
	SKILL_SPELL_ACTIVE_ATTACK_ICE_CHAIN,
	SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE,
	SKILL_SPELL_ACTIVE_ATTACK_ROOT,	
	SKILL_SPELL_ACTIVE_ATTACK_SLEEP,
	SKILL_SPELL_ACTIVE_ATTACK_WINTER_BINDING,
	SKILL_SPELL_ACTIVE_ATTACK_LIGHTNING_STRIKE,
	SKILL_SPELL_ACTIVE_ATTACK_HEL_FIRE,	
	SKILL_SPELL_ACTIVE_BUF_BACK_TELEPORT,
	SKILL_SPELL_ACTIVE_BUF_ILLUSION,

	SKILL_DISCIPLINE_ACTIVE_ATTACK_DIVINE_STRIKE,
	SKILL_DISCIPLINE_ACTIVE_ATTACK_THUNDER_BOLT,
	SKILL_DISCIPLINE_ACTIVE_ATTACK_ROOT,
	SKILL_DISCIPLINE_ACTIVE_ATTACK_JUDGMENT,
	SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_LIGHT,
	SKILL_DISCIPLINE_ACTIVE_HEAL_RECOVERY_LIGHT,
	SKILL_DISCIPLINE_ACTIVE_HEAL_VITALITY_LIGHT,
	SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_GRACE,
	SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_WIND,
	SKILL_DISCIPLINE_ACTIVE_HEAL_RECOVERY_WIND,

	SKILL_ASSASSINATION_ACTIVE_ATTACK_QUICK_CUT,
	SKILL_ASSASSINATION_ACTIVE_ATTACK_FAST_CUT,
	SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_ATTACK,
	SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_CUT,
	SKILL_ASSASSINATION_ACTIVE_ATTACK_ADVANCE_CUT,
	SKILL_ASSASSINATION_ACTIVE_ATTACK_POISON_INJECTION,
	SKILL_ASSASSINATION_ACTIVE_ATTACK_POISON_STUN,
	SKILL_ASSASSINATION_ACTIVE_ATTACK_ASSASSINATION,
	SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_STEP,
	SKILL_ASSASSINATION_ACTIVE_BUF_STEALTH,
	SKILL_ASSASSINATION_ACTIVE_BUF_SIXTH_SENSE_MAXIMIZE,
	SKILL_ASSASSINATION_ACTIVE_BUF_FOCUS_EVASION,

	SKILL_SHOOTING_ACTIVE_ATTACK_SNIFING,
	SKILL_SHOOTING_ACTIVE_ATTACK_CONTINUOUS_FIRE,
	SKILL_SHOOTING_ACTIVE_ATTACK_NOOSE_ARROW,
	SKILL_SHOOTING_ACTIVE_ATTACK_ASSAULT_ARROW,
	SKILL_SHOOTING_ACTIVE_ATTACK_IMPULSE_ARROW,
	SKILL_SHOOTING_ACTIVE_BUF_FOCUS_EVASION,		

	SKILL_GOBLIN_ACTIVE_MELEE_DEFAULT_ATTACK
};

enum class en_SkillCategory : int8
{
	SKILL_CATEGORY_NONE,
	SKILL_CATEGORY_GLOBAL,
	SKILL_CATEGORY_PASSIVE_SKILL,
	SKILL_CATEGORY_ACTIVE_SKILL,
	SKILL_CATEGORY_BUF_SKILL,
	SKILL_CATEGORY_STATUS_ABNORMAL_SKILL,	
	SKILL_CATEGORY_COMBO_SKILL
};

enum class en_SkillKinds : int8
{
	SKILL_KIND_NONE,
	SKILL_KIND_GLOBAL_SKILL,

	SKILL_KIND_MELEE_SKILL,	

	SKILL_KIND_SPELL_SKILL,	

	SKILL_KIND_HEAL_SKILL,	

	SKILL_KIND_RANGE_SKILL	
};

enum class en_SkillCharacteristic : int8
{
	SKILL_CATEGORY_NONE,
	SKILL_CATEGORY_PUBLIC,
	SKILL_CATEGORY_FIGHT,
	SKILL_CATEGORY_PROTECTION,	
	SKILL_CATEGORY_SPELL,
	SKILL_CATEGORY_DISCIPLINE,
	SKILL_CATEGORY_ASSASSINATION,
	SKILL_CATEGORY_SHOOTING	
};	

enum class en_UserQuickSlot : int16
{
	USER_KEY_QUICK_SLOT_NONE,

	USER_KEY_QUICK_SLOT_MOVE_UP,
	USER_KEY_QUICK_SLOT_MOVE_DOWN,
	USER_KEY_QUICK_SLOT_MOVE_LEFT,
	USER_KEY_QUICK_SLOT_MOVE_RIGHT,

	USER_KEY_QUICK_SLOT_UI_INVENTORY,
	USER_KEY_QUICK_SLOT_UI_SKILL_BOX,
	USER_KEY_QUICK_SLOT_UI_EQUIPMENT_BOX,

	USER_KEY_QUICK_SLOT_ONE_ONE,
	USER_KEY_QUICK_SLOT_ONE_TWO,
	USER_KEY_QUICK_SLOT_ONE_THREE,
	USER_KEY_QUICK_SLOT_ONE_FOUR,
	USER_KEY_QUICK_SLOT_ONE_FIVE,

	USER_KEY_QUICK_SLOT_TWO_ONE,
	USER_KEY_QUICK_SLOT_TWO_TWO,
	USER_KEY_QUICK_SLOT_TWO_THREE,
	USER_KEY_QUICK_SLOT_TWO_FOUR,
	USER_KEY_QUICK_SLOT_TWO_FIVE,

	USER_KEY_QUICK_SLOT_FIND_AROUND_OBJECT,

	USER_KEY_QUICK_SLOT_INTERACTION,	

	USER_KEY_ESCAPE,
	USER_KEY_ENTER_CHATTING
};

enum class en_KeyCode : int16
{
	KEY_CODE_NONE,

	KEY_CODE_W,
	KEY_CODE_CTRL_W,
	KEY_CODE_ALT_W,

	KEY_CODE_S,
	KEY_CODE_CTRL_S,
	KEY_CODE_ALT_S,

	KEY_CODE_A,
	KEY_CODE_CTAL_A,
	KEY_CODE_ALT_A,

	KEY_CODE_D,
	KEY_CODE_CTRL_D,
	KEY_CODE_ALT_D,

	KEY_CODE_I,
	KEY_CODE_CTRL_I,
	KEY_CODE_ALT_I,

	KEY_CODE_E,
	KEY_CODE_CTRL_E,
	KEY_CODE_ALT_E,

	KEY_CODE_K,
	KEY_CODE_CTRL_K,
	KEY_CODE_ALT_K,

	KEY_CODE_ONE,
	KEY_CODE_CTRL_ONE,
	KEY_CODE_ALT_ONE,

	KEY_CODE_TWO,
	KEY_CODE_CTRL_TWO,
	KEY_CODE_ALT_TWO,

	KEY_CODE_THREE,
	KEY_CODE_CTRL_THREE,
	KEY_CODE_ALT_THREE,

	KEY_CODE_FOUR,
	KEY_CODE_CTRL_FOUR,
	KEY_CODE_ALT_FOUR,

	KEY_CODE_FIVE,
	KEY_CODE_CTRL_FIVE,
	KEY_CODE_ALT_FIVE,

	KEY_CODE_SIX,
	KEY_CODE_CTRL_SIX,
	KEY_CODE_ALT_SIX,

	KEY_CODE_SEVEN,
	KEY_CODE_CTRL_SEVEN,
	KEY_CODE_ALT_SEVEN,

	KEY_CODE_EIGHT,
	KEY_CODE_CTRL_EIGHT,
	KEY_CODE_ALT_EIGHT,

	KEY_CODE_NINE,
	KEY_CODE_CTRL_NINE,
	KEY_CODE_ALT_NINE,

	KEY_CODE_ZERO,
	KEY_CODE_CTRL_ZERO,
	KEY_CODE_ALT_ZERO,

	KEY_CODE_Q,
	KEY_CODE_CTRL_Q,
	KEY_CODE_ALT_Q,

	KEY_CODE_R,
	KEY_CODE_CTRL_R,
	KEY_CODE_ALT_R,

	KEY_CODE_T,
	KEY_CODE_CTRL_T,
	KEY_CODE_ALT_T,

	KEY_CODE_Y,
	KEY_CODE_CTRL_Y,
	KEY_CODE_ALT_Y,

	KEY_CODE_U,
	KEY_CODE_CTRL_U,
	KEY_CODE_ALT_U,

	KEY_CODE_O,
	KEY_CODE_CTRL_O,
	KEY_CODE_ALT_O,

	KEY_CODE_P,
	KEY_CODE_CTRL_P,
	KEY_CODE_ALT_P,

	KEY_CODE_F,
	KEY_CODE_CTRL_F,
	KEY_CODE_ALT_F,

	KEY_CODE_G,
	KEY_CODE_CTRL_G,
	KEY_CODE_ALT_G,

	KEY_CODE_H,
	KEY_CODE_CTRL_H,
	KEY_CODE_ALT_H,

	KEY_CODE_J,
	KEY_CODE_CTRL_J,
	KEY_CODE_ALT_J,

	KEY_CODE_L,
	KEY_CODE_CTRL_L,
	KEY_CODE_ALT_L,

	KEY_CODE_Z,
	KEY_CODE_CTRL_Z,
	KEY_CODE_ALT_Z,

	KEY_CODE_X,
	KEY_CODE_CTRL_X,
	KEY_CODE_ALT_X,

	KEY_CODE_C,
	KEY_CODE_CTRL_C,
	KEY_CODE_ALT_C,

	KEY_CODE_V,
	KEY_CODE_CTRL_V,
	KEY_CODE_ALT_V,

	KEY_CODE_B,
	KEY_CODE_CTRL_B,
	KEY_CODE_ALT_B,

	KEY_CODE_N,
	KEY_CODE_CTRL_N,
	KEY_CODE_ALT_N,

	KEY_CODE_M,
	KEY_CODE_CTRL_M,
	KEY_CODE_ALT_M,

	KEY_CODE_CAPSLOCK,
	KEY_CODE_CTRL_CAPSLOCK,
	KEY_CODE_ALT_CAPSLOCK,

	KEY_CODE_ESCAPE,
	KEY_CODE_ENTER,
	KEY_CODE_TAB
};

enum class en_EquipmentParts : int8
{
	EQUIPMENT_PARTS_NONE,
	EQUIPMENT_PARTS_HEAD,
	EQUIPMENT_PARTS_BODY,
	EQUIPMENT_PARTS_LEFT_HAND,
	EQUIPMENT_PARTS_RIGHT_HAND,	
	EQUIPMENT_PARTS_BOOT
};

enum class en_ResourceName : int16
{
	CLIENT_UI_NAME_NONE = 0,

	CLIENT_GAMEOBJECT_PLAYER,

	CLIENT_GAMEOBJECT_NON_PLAYER_GENERAL_MERCHANT,

	CLIENT_GAMEOBJECT_MONSTER_GOBLIN,

	CLIENT_GAMEOBJECT_LEFT_RIGHT_WALL,
	CLIENT_GAMEOBJECT_UP_DOWN_WALL,
	CLIENT_GAMEOBJECT_UP_TO_LEFT_WALL,
	CLIENT_GAMEOBJECT_UP_TO_RIGHT_WALL,
	CLIENT_GAMEOBJECT_DOWN_TO_LEFT_WALL,
	CLIENT_GAMEOBJECT_DOWN_TO_RIGHT_WALL,

	CLIENT_GAMEOBJECT_ENVIRONMENT_STONE,
	CLIENT_GAMEOBJECT_ENVIRONMENT_TREE,

	CLIENT_GAMEOBJECT_CROP_POTATO,
	CLIENT_GAMEOBJECT_CROP_CORN,

	CLIENT_GAMEOBJECT_SKILL_SWORD_BLADE,
	CLIENT_GAMEOBJECT_SKILL_FLAME_BOLT,
	CLIENT_GAMEOBJECT_SKILL_DIVINE_BOLT,

	CLIENT_GAMEOBJECT_CRAFTING_TABLE_FURNACE,
	CLIENT_GAMEOBJECT_CRAFTING_TABLE_SAWMILL,

	CLIENT_WEAPON_PARENT,
	CLIENT_WEAPON_DAGGER_WOOD,
	CLIENT_WEAPON_LONG_SWORD_WOOD,
	CLIENT_WEAPON_GREAT_SWORD_WOOD,
	CLIENT_WEAPON_SHIELD_WOOD,
	CLIENT_WEAPON_BOW_WOOD,

	CLIENT_GAMEOBJECT_STORAGE_BOX_SMALL,

	CLIENT_GAMEOBJECT_DAY,

	CLIENT_GAMEOBJECT_ITEM_LEATHER,
	CLIENT_GAMEOBJECT_ITEM_BRONZE_COIN,
	CLIENT_GAMEOBJECT_ITEM_WOOD_LOG,
	CLIENT_GAMEOBJECT_ITEM_STONE,
	CLIENT_GAMEOBJECT_ITEM_WOOD_FLANK,
	CLIENT_GAMEOBJECT_ITEM_CHARCOAL,
	CLIENT_GAMEOBJECT_ITEM_POTATO,

	CLIENT_EFFECT_ATTACK_TARGET_HIT,
	CLIENT_EFFECT_SMASH_WAVE,
	CLIENT_EFFECT_CHOHONE,
	CLIENT_EFFECT_SHAHONE,
	CLIENT_EFFECT_CHARGE_POSE,
	CLIENT_EFFECT_FLAME_HARPOON,
	CLIENT_EFFECT_LIGHTHING,
	CLIENT_EFFECT_BACK_TELEPORT,
	CLIENT_EFFECT_HEALING_LIGHT,
	CLIENT_EFFECT_HEALING_WIND,
	CLIENT_EFFECT_STUN,

	CLIENT_COLLISION_RECT,

	CLIENT_UI_SERVER_SELECT_ITEM,

	CLIENT_UI_CHARACTER_CHOICE_ITEM,

	CLIENT_UI_CHATTING_BOX,
	CLIENT_UI_CHATTING_TEXT,

	CLIENT_UI_CRAFTING_CATEGORY_ITEM,
	CLIENT_UI_CRAFTING_MATERIAL_ITEM,
	CLIENT_UI_CRAFTING_COMPLETE_ITEM,

	CLIENT_UI_QUICK_SLOT_BAR_BOX,
	CLIENT_UI_QUICK_SLOT_BAR,
	CLIENT_UI_QUICK_SLOT_BAR_BUTTON,
	CLIENT_UI_QUICK_SLOT_ITEM_DRAG,

	CLIENT_UI_QUICK_SLOT_KEY,
	CLIENT_UI_QUICK_SLOT_KEY_ITEM,

	CLIENT_UI_INVENTORY_BOX,
	CLIENT_UI_INVENTORY_ITEM,
	CLIENT_UI_INVENTORY_ITEM_DRAG,
	CLIENT_UI_INVENTORY_ITEM_GAIN,

	CLIENT_UI_PARTY,
	CLIENT_UI_PARTY_PLAYER_INFO_FRAME,
	CLIENT_UI_PARTY_PLAYER_OPTION,
	CLIENT_UI_PARTY_REACTION,

	CLIENT_UI_GLOBAL_MESSAGE,

	CLIENT_UI_DAMAGE,

	CLIENT_UI_EQUIPMENT_BOX,
	CLIENT_UI_EQUIPMENT_ITEM,

	CLIENT_UI_SKILL_BOX,
	CLIENT_UI_SKILL_EXPLANATION,
	CLIENT_UI_SKILL_ITEM,
	CLIENT_UI_SKILL_ITEM_DRAG,
	CLIENT_UI_SKILL_BUF_DEBUF,

	CLIENT_UI_ITEM_EXPLANATION,
	CLIENT_UI_ITEM_DIVIDE,

	CLIENT_UI_CRAFTING_BOX,
	CLIENT_UI_FURNACE,
	CLIENT_UI_SAWMILL,

	CLIENT_UI_OPTION,
	CLIENT_UI_OPTION_ITEM,

	CLIENT_UI_MY_CHARACTER_HUD,
	CLIENT_UI_TARGET_HUD,
	CLIENT_UI_PLAYER_OPTION,

	CLIENT_UI_HP_BAR,
	CLIENT_UI_SKILL_CASTING_BAR,
	CLIENT_UI_GATHERING_BAR,

	CLIENT_UI_NAME,

	CLIENT_UI_SPEECH_BUBBLE,

	CLIENT_UI_INTERACTION,

	CLIENT_UI_SCENE_LOGIN,
	CLIENT_UI_SCENE_GAME,

	CLIENT_UI_EVENT_SYTEM,

	CLIENT_MAP_MAIN_FIELD,
	CLIENT_MAP_TILE
};

enum class en_CommonErrorType : int16
{
	ERROR_STATUS_ABNORMAL_MOVE,
	ERROR_STATUS_ABNORMAL_MELEE,
	ERROR_STATUS_ABNORMAL_MAGIC
};

enum class en_GlobalMessageType : int8
{
	GLOBAL_MESSAGE_NONE,
	
	GLOBAL_MESSAGE_STATUS_ABNORMAL,	
	GLOBAL_MESSAGE_STATUS_ABNORMAL_FIGHT_JUMPING_ATTACK,
	
	GLOBAL_MESSAGE_STATUS_ABNORMAL_PROTECTION_CAPTURE,
	
	GLOBAL_MESSAGE_STATUS_ABNORMAL_SPELL_ROOT,
	GLOBAL_MESSAGE_STATUS_ABNORMAL_SPELL_ICE_CHAIN,
	GLOBAL_MESSAGE_STATUS_ABNORMAL_SPELL_ICE_WAVE,
	GLOBAL_MESSAGE_STATUS_ABNORMAL_SPELL_LIGHTNING_STRIKE,
	
	GLOBAL_MESSAGE_STATUS_ABNORMAL_DISCIPLINE_ROOT,
	
	GLOBAL_MESSAGE_NON_SKILL_CHARACTERISTIC,
	GLOBAL_MESSAGE_SKILL_COOLTIME,
	GLOBAL_MESSAGE_GLOBAL_SKILL_COOLTIME,
	GLOBAL_MESSAGE_SKILL_CANCEL_FAIL_COOLTIME,
	GLOBAL_MESSAGE_NON_SELECT_OBJECT,
	GLOBAL_MESSAGE_HEAL_NON_SELECT_OBJECT,
	GLOBAL_MESSAGE_PLACE_BLOCK,
	GLOBAL_MESSAGE_PLACE_DISTANCE,
	GLOBAL_MESSAGE_FAR_DISTANCE,
	GLOBAL_MESSAGE_MYSELF_TARGET,
	GLOBAL_MESSAGE_ATTACK_ANGLE,	
	
	GLOBAL_MESSAGE_DIR_DIFFERENT,
	GLOBAL_MESSAGE_GATHERING_DISTANCE,
	
	GLOBAL_MEESAGE_CRAFTING_TABLE_OVERLAP_SELECT,
	GLOBAL_MESSAGE_CRAFTING_TABLE_OVERLAP_CRAFTING_START,
	GLOBAL_MESSAGE_CRAFTING_TABLE_MATERIAL_COUNT_NOT_ENOUGH,
	GLOBAL_MESSAGE_CRAFTING_TABLE_MATERIAL_WRONG_ITEM_ADD,
	
	GLOBAL_MESSAGE_LOGIN_ACCOUNT_NOT_EXIST,
	GLOBAL_MESSAGE_LOGIN_ACCOUNT_OVERLAP,
	GLOBAL_MESSAGE_LOGIN_ACCOUNT_DB_WORKING,
	GLOBAL_MESSAGE_LOGIN_ACCOUNT_DIFFERENT_PASSWORD,
	
	GLOBAL_MESSAGE_SEED_FARMING_EXIST,
	
	GLOBAL_MESSAGE_EXIST_PARTY_PLAYER,
	GLOBAL_MESSAGE_PARTY_INVITE_REJECT,
	GLOBAL_MESSAGE_PARTY_MAX,
	
	GLOBAL_FAULT_ITEM_USE,
	
	GLOBAL_UI_CLOSE
};

enum class en_ConsumableType : int16
{
	NONE,
	POTION	
};

enum class en_GameObjectJobType : int16
{	
	GAMEOBJECT_JOB_TYPE_MOVE,
	GAMEOBJECT_JOB_TYPE_MOVE_STOP,	
	GAMEOBJECT_JOB_TYPE_LOOK_AT_DIRECTION,
	GAMEOBJECT_JOB_TYPE_SELECT_SKILL_CHARACTERISTIC,
	GAMEOBJECT_JOB_TYPE_SKILL_LEARN,
	GAMEOBJECT_JOB_TYPE_SKILL_PROCESS,
	GAMEOBJECT_JOB_TYPE_SKILL_CASTING_CANCEL,		
	GAMEOBJECT_JOB_TYPE_COMBO_SKILL_CREATE,

	GAMEOBJECT_JOB_TYPE_GATHERING_START,
	GAMEOBJECT_JOB_TYPE_GATHERING_CANCEL,	

	GAMEOBJECT_JOB_TYPE_AGGRO_LIST_INSERT_OR_UPDATE,
	GAMEOBJECT_JOB_TYPE_AGGRO_LIST_REMOVE,	

	GAMEOBJECT_JOB_TYPE_DAMAGE,	
	GAMEOJBECT_JOB_TYPE_SKILL_HP_HEAL,
	GAMEOJBECT_JOB_TYPE_SKILL_MP_HEAL,
	GAMEOBJECT_JOB_TYPE_ITEM_HP_HEAL,
	GAMEOBJECT_JOB_TYPE_ITEM_MP_HEAL,

	GAMEOBJECT_JOB_TYPE_ON_EQUIPMENT,	
	GAMEOBJECT_JOB_TYPE_OFF_EQUIPMENT,	

	GAMEOBJECT_JOB_TYPE_INTERACTION,

	GAMEOBJECT_JOB_TYPE_CHANNEL_PARTY_INVITE,
	GAMEOBJECT_JOB_TYPE_CHANNEL_PARTY_INVITE_ACCEPT,
	GAMEOBJECT_JOB_TYPE_CHANNEL_PARTY_INVITE_REJECT,
	GAMEOBJECT_JOB_TYPE_CHANNEL_PARTY_QUIT,
	GAMEOBJECT_JOB_TYPE_CHANNEL_PARTY_BANISH,
	GAMEOBJECT_JOB_TYPE_CHANNEL_PARTY_LEADER_MANDATE,	
	GAMEOBJECT_JOB_TYPE_CHANNEL_MENU,	
	GAMEOBJECT_JOB_TYPE_CHANNEL_OBJECT_SPAWN,
	GAMEOBJECT_JOB_TYPE_CHANNEL_OBJECT_DESPAWN,
	GAMEOBJECT_JOB_TYPE_CHANNEL_PLAYER_ENTER,
	GAMEOBJECT_JOB_TYPE_CHANNEL_OBJECT_ENTER,
	GAMEOBJECT_JOB_TYPE_CHANNEL_OBJECT_LEAVE,
	GAMEOBJECT_JOB_TYPE_CHANNEL_PLAYER_LEAVE,
	GAMEOBJECT_JOB_TYPE_CHANNEL_FIND_OBJECT,
	GAMEOBJECT_JOB_TYPE_CHANNEL_CRAFTING_TABLE_SELECT_ITEM,
	GAMEOBJECT_JOB_TYPE_CHANNEL_CRAFTING_TABLE_NON_SELECT,
	GAMEOBJECT_JOB_TYPE_CHANNEL_RIGHT_MOUSE_OBJECT_INFO,
	GAMEOBJECT_JOB_TYPE_CHANNEL_LEFT_MOUSE_DRAG_OBJECTS_SELECT,
	GAMEOBJECT_JOB_TYPE_CHANNEL_SEED_FARMING,
	GAMEOBJECT_JOB_TYPE_CHANNEL_PLANT_GROWTH_CHECK,	

	GAMEOBJECT_JOB_TYPE_FULL_RECOVERY,

	GAMEOBJECT_JOB_TYPE_ITEM_DROP,
	GAMEOBJECT_JOB_TYPE_ITEM_INVENTORY_SAVE,
	GAMEOBJECT_JOB_TYPE_CRAFTING_TABLE_ITEM_ADD,
	GAMEOBJECT_JOB_TYPE_CRAFTING_TABLE_MATERIAL_ITEM_SUBTRACT,
	GAMEOBJECT_JOB_TYPE_CRAFTING_TABLE_COMPLETE_ITEM_SUBTRACT,
	GAMEOBJECT_JOB_TYPE_CRAFTING_TABLE_CRAFTING_START,
	GAMEOBJECT_JOB_TYPE_CRAFTING_TABLE_CRAFTING_STOP
};

enum class en_MonsterAggroType : int8
{
	MONSTER_AGGRO_FIRST_TARGET,
	MONSTER_AGGRO_SECOND_TARGET,
	MONSTER_AGGRO_FIRST_ATTACKER,
	MONSTER_AGGRO_DAMAGE,
	MONSTER_AGGRO_HEAL,
	MONSTER_AGGRO_BUF,
	MONSTER_AGGRO_DEBUF,
	MONSTER_AGGRO_TAUNT
};

enum class en_AggroCategory : int8
{
	AGGRO_CATEGORY_DAMAGE,
	AGGRO_CATEGORY_HEAL
};

// 한 타일에 존재 할 수 있는 아이템의 최대 종류 개수
enum class en_MapItemInfo : int8
{
	MAP_ITEM_COUNT_MAX = 20
};

enum class en_OptionType : int8
{
	OPTION_TYPE_NONE = 0,
	OPTION_TYPE_TILE_BUY
};

enum class en_DayType : int8
{
	DAY_NONE = 0,
	DAY_DAWN,
	DAY_MORNING,
	DAY_AFTERNOON,
	DAY_EVENING,
	DAY_NIGHT
};

enum class en_QuickSlotBarType : int8
{
	QUICK_SLOT_BAR_TYPE_NONE = 0,
	QUICK_SLOT_BAR_TYPE_SKILL,
	QUICK_SLOT_BAR_TYPE_ITEM
};

enum class en_Season : int8
{
	SEASON_NONE,
	SEASON_SPRING,
	SEASON_SUMMER,
	SEASON_FALL,
	SEASON_WINTER
};

enum class en_WallType : int8
{
	WALL_TYPE_NONE,
	WALL_TYPE_COPPER,
	WALL_TYPE_SILVER,
	WALL_TYPE_GOLD,
	WALL_TYPE_STONE,
	WALL_TYPE_IRON
};

enum class en_AnimationType : int8
{
	ANIMATION_TYPE_IDLE,
	ANIMATION_TYPE_SWORD_MELEE_ATTACK
};

enum class en_CollisionPosition : int8
{
	COLLISION_POSITION_NONE,
	COLLISION_POSITION_OBJECT,
	COLLISION_COMMON_MIDDLE,
	COLLISION_POSITION_SKILL_MIDDLE,
	COLLISION_POSITION_SKILL_FRONT
};

enum class en_MenuType : int8
{
	MENU_TYPE_NONE,
	MENU_TYPE_CHARACTER_CHOICE
};

enum class en_InteractionType : int8
{
	INTERACTION_TYPE_NONE,
	INTERACTION_TYPE_ROOTING
};

enum class en_TileInfo
{
	TILE_INFO_NONE,

	TILE_INFO_EMPTY_TILE,
	TILE_INFO_SYSTEM_WALL,
	TILE_INFO_GOVERNMENTOFFICE_TILE,
	TILE_INFO_WEAPON_STORE_TILE,
	TILE_INFO_ARMOR_STORE_TILE
};


enum class en_WorldMapInfo : int8
{
	WORLD_MAP_NONE,
	WORLD_MAP_MAIN_FIELD
};

struct st_BindingKey
{
	en_UserQuickSlot UserQuickSlot;
	en_KeyCode KeyCode;
};

struct st_RayCatingPosition
{
	Vector2 StartPosition;
	Vector2 EndPosition;
};

struct st_PositionInfo
{
	en_CreatureState State;
	Vector2Int CollisionPosition;
	Vector2 Position;
	Vector2 LookAtDireciton;
	Vector2 MoveDirection;
};

struct st_StatInfo
{
	int8 Level = 0;
	
	int32 Str = 0; // 무력
	int32 Int = 0; // 지력
	int32 Dex = 0; // 민첩
	int32 Luck = 0; // 행운

	int32 Con = 0; // 체력
	int32 Wis = 0; // 지혜

	int16 Stamina = 0; // 활동력

	int32 HP = 0; // 현재 생명력
	int32 MaxHP = 0; // 최대 생명력
	int32 MP = 0; // 현재 마력
	int32 MaxMP = 0; // 최대 마력 

	int16 AutoRecoveryHPPercent = 0; // 체력 자동 회복 비율
	int16 AutoRecoveryMPPercent = 0; // 마력 자동 회복 비율

	// Str을 1차 능력치로 해서 계산
	int32 MinAttackPoint = 0; // 최소 공격력
	int32 MaxAttackPoint = 0; // 최대 공격력

	// 지력을 1차 능력치로 해서 계산
	int16 SpiritPoint = 0; // 정신력

	// 무기에 따라 공격 속도 달라짐
	int16 AttackHitRate = 0; // 공격 속도
	float SpellCastingRate = 0; // 시전 속도
	
	int32 Defence = 0;  // 방어력

	// 민첩을 1차 능력치로 해서 계산
	int16 EvasionRate = 0; // 회피율

	// 행운을 1차 능력치로 해서 물리 치명타율과 주문치명타율 계산
	int16 AttackCriticalPoint = 0; // 물리 치명타율	
	int16 SpellCriticalPoint = 0;  // 주문 치명타율

	int16 AttackCriticalResistance = 0; // 물리 치명타 저항율

	int16 StatusAbnormalResistance = 0; // 상태이상 저항률

	float Speed = 0; // 속도
	float MaxSpeed = 0; // 최대 속도

	float SearchDistance = 0;
	float ChaseDistance = 0;
	float MovingAttackRange = 0;
	float AttackRange = 0;
};

struct st_Experience
{
	int64 CurrentExperience;
	int64 RequireExperience;
	int64 TotalExperience;
	float CurrentExpRatio;

	st_Experience()
	{
		CurrentExperience = 0;
		RequireExperience = 0;
		TotalExperience = 0;
		CurrentExpRatio = 0;
	}
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
	static st_Color SkyBlue() { return st_Color(80, 188, 223); }
	static st_Color Yellow() { return st_Color(255, 212, 255); }
	static st_Color White() { return st_Color(255, 255, 255); }
};

struct st_CraftingMaterialItemInfo
{
	en_SmallItemCategory MaterialItemType; // 재료템 종류
	wstring MaterialItemName; // 재료템 이름
	int16 ItemCount; // 재료템 필요 개수
};

struct st_CraftingCompleteItem
{
	en_UIObjectInfo OwnerCraftingTable;  // 완성 제작템을 소유한 제작대
	en_SmallItemCategory CompleteItemType; // 완성 제작템 종류
	wstring CompleteItemName; // 완성 제작템 이름
	vector<st_CraftingMaterialItemInfo> Materials; // 제작템 만들때 필요한 재료들
};

struct st_CraftingItemCategory
{
	en_LargeItemCategory CategoryType; // 제작템 범주
	wstring CategoryName; // 제작템 범주 이름
	vector<CItem*> CommonCraftingCompleteItems; // 범주에 속한 완성 제작템들
};

struct st_ItemInfo
{
	int64 ItemDBId;							  // 아이템 DB에 저장되어 있는 ID		
	int64 InventoryItemNumber;				  // 아이템이 인벤토리에 속할때 구분할 숫자	
	bool ItemIsEquipped;			          // 아이템을 착용할 수 있는지		
	bool ItemIsSearching;					  // 탐색이 완료 된 아이템인지 여부
	int16 ItemWidth;			     		  // 아이템 너비
	int16 ItemHeight;						  // 아이템 높이	
	float ItemCollisionX;					  // 장비 아이템 감지 박스 X 크기
	float ItemCollisionY;					  // 장비 아이템 감지 박스 Y 크기
	int16 ItemTileGridPositionX;			  // 인벤토리 위치 X
	int16 ItemTileGridPositionY;			  // 인벤토리 위치 Y
	int32 ItemSearchingTime;				  // 아이템 탐색 시간
	int64 ItemSearchingRemainTime;			  // 아이템 탐색 남은 시간
	en_UIObjectInfo OwnerCraftingTable;		  // 아이템이 제작 가능한 아이템이라면 아이템이 속한 제작대
	en_LargeItemCategory ItemLargeCategory;   // 아이템 대분류
	en_MediumItemCategory ItemMediumCategory; // 아이템 중분류
	en_SmallItemCategory ItemSmallCategory;	  // 아이템 소분류
	en_GameObjectType ItemObjectType;		  // 아이템 ObjectType
	en_SkillLargeCategory ItemSkillLargeCategory; // 아이템 스킬 대분류
	en_SkillMediumCategory ItemSkillMediumCategory; // 아이템 스킬 중분류
	en_SkillType ItemSkillType;				  // 아이템 스킬 소분류
	en_EquipmentParts ItemEquipmentPart;      // 아이템 장비 분류
	int32 ItemMaxDurability;				  // 아이템 최대 내구도
	int32 ItemCurrentDurability;			  // 아이템 현재 내구도	
	int8 ItemMaxGatheringHP;				  // 아이템 최대 채집 내구도 ( 채집물을 채집할 때 소모되는 점수 값 )
	int8 ItemCurrentGatheringHP;			  // 아이템 현재 채집 내구도
	wstring ItemName;			              // 아이템 이름
	wstring ItemExplain;		              // 아이템 설명문
	int64 ItemCraftingTime;					  // 아이템 제작 시간
	int64 ItemCraftingRemainTime;			  // 아이템 제작 남은 시간	
	int32 ItemMinDamage;			          // 아이템 최소 공격력
	int32 ItemMaxDamage;			          // 아이템 최대 공격력
	int32 ItemDefence;				          // 아이템 방어력
	int16 ItemHealPoint;					  // 아이템 체력 회복 점수
	int32 ItemMaxCount;				          // 아이템을 소유 할 수 있는 최대 개수
	int16 ItemCount;			              // 개수				
	int8 ItemMaxstep;						  // 아이템 단계 최대값
	int32 ItemGrowTime;						  // 작물 성장 시간
	int8 ItemEnchantPoint;					  // 아이템 강화 수치
	vector<st_CraftingMaterialItemInfo> Materials; // 제작 아이템일 경우 조합에 필요한 재료 아이템 목록		

	st_ItemInfo()
	{
		ItemDBId = 0;
		ItemIsSearching = false;
		InventoryItemNumber = 0;		
		ItemWidth = 0;
		ItemHeight = 0;		
		ItemTileGridPositionX = 0;
		ItemTileGridPositionY = 0;
		OwnerCraftingTable = en_UIObjectInfo::UI_OBJECT_INFO_NONE;
		ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_NONE;
		ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_NONE;
		ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_NONE;
		ItemObjectType = en_GameObjectType::OBJECT_NON_TYPE;
		ItemSkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_NONE;
		ItemSkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_NONE;
		ItemSkillType = en_SkillType::SKILL_TYPE_NONE;
		
		ItemEquipmentPart = en_EquipmentParts::EQUIPMENT_PARTS_NONE;

		ItemMaxDurability = 0;
		ItemCurrentDurability = 0;

		ItemMaxGatheringHP = 0;
		ItemCurrentGatheringHP = 0;

		ItemName = L"";

		ItemExplain = L"";

		ItemCraftingTime = 0;
		ItemCraftingRemainTime = 0;

		ItemSearchingTime = 0;
		ItemSearchingRemainTime = 0;

		ItemMinDamage = 0;
		ItemMaxDamage = 0;

		ItemDefence = 0;

		ItemHealPoint = 0;

		ItemMaxCount = 0;
		ItemCount = 0;				

		ItemMaxstep = 0;
		ItemGrowTime = 0;

		ItemIsEquipped = false;

		ItemEnchantPoint = 0;
	}

	bool operator == (st_ItemInfo OtherItemInfo)
	{
		if (ItemTileGridPositionX == OtherItemInfo.ItemTileGridPositionX
			&& ItemTileGridPositionY == OtherItemInfo.ItemTileGridPositionY
			&& ItemLargeCategory == OtherItemInfo.ItemLargeCategory
			&& ItemMediumCategory == OtherItemInfo.ItemMediumCategory
			&& ItemSmallCategory == OtherItemInfo.ItemSmallCategory
			&& ItemName == OtherItemInfo.ItemName
			&& ItemCount == OtherItemInfo.ItemCount)
		{
			return true;
		}

		return false;
	}

	bool operator != (st_ItemInfo OtherItemInfo)
	{
		return !(*this == OtherItemInfo);
	}
};

struct st_SkillInfo
{
	bool IsSkillLearn;       // 스킬을 배웠는지에 대한 여부
	bool CanSkillUse;		 // 스킬을 사용 할 수 있는지 여부	
	en_GameObjectStatusType SkillStatusAbnormal; // 스킬의 상태이상 적용 값
	en_GameObjectStatusType SkillStatusAbnormalMask; // 스킬의 상태이상 해제 적용 값
	en_SkillKinds SkillKind; // 스킬 데미지 타입 종류 ( 물리, 마법, 원거리 )	
	en_SkillCharacteristic SkillCharacteristic; // 스킬 특성
	en_SkillLargeCategory SkillLargeCategory; // 스킬 대분류
	en_SkillMediumCategory SkillMediumCategory; // 스킬 중분류
	en_SkillType SkillType;	 // 스킬 종류
	int8 SkillNumber;		 // 스킬 배치 순서
	int8 SkillMaxLevel;	     // 스킬 최대 레벨
	int8 SkillLevel;		 // 스킬 레벨
	int8 SkillOverlapStep;   // 스킬 중첩 횟수
	wstring SkillName;		 // 스킬 이름	
	float SkillDistance;	 // 스킬 유효 거리
	float SkillRangeX;		 // 스킬 범위 X 크기
	float SkillRangeY;		 // 스킬 범위 Y 크기	
	int32 SkillCoolTime;	 // 스킬 쿨타임		
	int32 SkillCastingTime;  // 스킬 캐스팅 타임
	int64 SkillDurationTime; // 스킬 지속 시간
	int64 SkillDotTime;      // 스킬 도트 시간 
	int64 SkillRemainTime;   // 스킬 남은 시간
	int32 SkillMotionTime;	 // 스킬 모션 시간
	float SkillTargetEffectTime; // 스킬 이펙트 시간
	int32 SkillMinHealPoint; // 최소 치유량
	int32 SkillMaxHealPoint; // 최대 치유량
	bool SkillIsDamage;		 // 피해량 적용 여부
	int32 SkillMinDamage;		// 최소 공격력
	int32 SkillMaxDamage;		// 최대 공격력	
	bool SkillDebuf;			// 스킬 디버프 여부	
	float SkillDebufAttackSpeed; // 스킬 공격속도 감소 수치
	float SkillDebufMovingSpeed; // 스킬 이동속도 감소 수치
	bool SkillDebufStun;		// 스킬 스턴 여부
	bool SkillDebufPushAway;	// 스킬 밀려남 여부
	bool SkillDebufRoot;		// 스킬 이동불가 여부	
	int64 SkillDamageOverTime;	// 스킬 도트 데미지 시간 간격
	int8 StatusAbnormalityProbability; // 상태 이상 적용 확률
	int32 IncreaseMinAttackPoint; // 증가하는 최소 근접 공격력
	int32 IncreaseMaxAttackPoint; // 증가하는 최대 근접 공격력
	int32 IncreaseMeleeAttackSpeedPoint; // 증가하는 근접 공격 속도
	int16 IncreaseMeleeAttackHitRate; // 증가하는 근접 명중률	
	int16 IncreaseMagicAttackPoint; // 증가하는 마법 공격력
	int16 IncreaseMagicCastingPoint; // 증가하는 마법 캐스팅 속도
	int16 IncreaseMagicAttackHitRate; // 증가하는 마법 명중률		
	int32 IncreaseDefencePoint; // 증가하는 방어력 
	int16 IncreaseEvasionRate; // 증가하는 회피율
	int16 IncreaseMeleeCriticalPoint; // 증가하는 근접 치명타율
	int16 IncreaseMagicCriticalPoint; // 증가하는 마법 치명타율
	float IncreaseSpeedPoint; // 증가하는 이동 속도	
	int16 IncreaseStatusAbnormalityResistance; // 증가하는 상태이상저항값
	en_SkillType NextComboSkill; // 다음 연속기 스킬		
	en_SkillType RollBackSkill; // 연속기 스킬일 경우 되돌려질 스킬

	st_SkillInfo()
	{
		IsSkillLearn = false;
		CanSkillUse = true;
		SkillNumber = 0;
		SkillStatusAbnormal = en_GameObjectStatusType::STATUS_ABNORMAL_NONE;
		SkillKind = en_SkillKinds::SKILL_KIND_NONE;
		SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_NONE;
		SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_NONE;
		SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_NONE;
		SkillType = en_SkillType::SKILL_TYPE_NONE;
		SkillMaxLevel = 0;
		SkillLevel = 1;
		SkillOverlapStep = 0;
		SkillName = L"";
		SkillDistance = 0;
		SkillRangeX = 0;
		SkillRangeY = 0;		
		SkillCoolTime = 0;		
		SkillCastingTime = 0;
		SkillDurationTime = 0;
		SkillDotTime = 0;
		SkillRemainTime = 0;
		SkillMotionTime = 0;
		SkillTargetEffectTime = 0;
		SkillMinHealPoint = 0;
		SkillMaxHealPoint = 0;
		SkillIsDamage = false;
		SkillMinDamage = 0;
		SkillMaxDamage = 0;
		SkillDebuf = false;
		SkillDebufAttackSpeed = 0;
		SkillDebufMovingSpeed = 0;
		SkillDebufStun = false;
		SkillDebufPushAway = false;
		SkillDebufRoot = false;
		SkillDamageOverTime = 0;
		StatusAbnormalityProbability = 0;
		IncreaseMinAttackPoint = 0;
		IncreaseMaxAttackPoint = 0;
		IncreaseMeleeAttackSpeedPoint = 0;
		IncreaseMeleeAttackHitRate = 0;
		IncreaseMagicAttackPoint = 0;
		IncreaseMagicCastingPoint = 0;
		IncreaseMagicAttackHitRate = 0;
		IncreaseDefencePoint = 0;
		IncreaseEvasionRate = 0;
		IncreaseMeleeCriticalPoint = 0;
		IncreaseMagicCriticalPoint = 0;
		IncreaseSpeedPoint = 0;
		IncreaseStatusAbnormalityResistance = 0;
		NextComboSkill = en_SkillType::SKILL_TYPE_NONE;	
		RollBackSkill = en_SkillType::SKILL_TYPE_NONE;
	}
};

struct st_QuickSlotBarSlotInfo
{
	en_QuickSlotBarType QuickSlotBarType; // 퀵슬롯이 담고 있는
	int64 AccountDBId;					 // 퀵슬롯 슬롯 소유한 Account
	int64 PlayerDBId;			         // 퀵슬롯 슬롯 소유한 Player	
	int8 QuickSlotBarIndex;			     // 퀵슬롯 Index
	int8 QuickSlotBarSlotIndex;			 // 퀵슬롯 슬롯 Index	
	CSkill* QuickBarSkill = nullptr;	 // 퀵슬롯에 등록할 스킬 정보		
	CItem* QuickBarItem = nullptr;		 // 퀵슬롯에 등록할 아이템 정보
	bool CanQuickSlotUse = true;		 // 퀵슬롯을 사용할 수 있는지 없는지
};

struct st_QuickSlotBarPosition
{
	int8 QuickSlotBarIndex;
	int8 QuickSlotBarSlotIndex;

	bool operator == (st_QuickSlotBarPosition QuickSlotInfo)
	{
		if (QuickSlotBarIndex == QuickSlotInfo.QuickSlotBarIndex
			&& QuickSlotBarSlotIndex == QuickSlotInfo.QuickSlotBarSlotIndex)
		{
			return true;
		}

		return false;
	}
};

struct st_QuickSlotOffInfo
{
	int8 QuickSlotBarIndex;
	int8 QuickSlotBarSlotIndex;
	en_SkillType RollBackSkillType;
};



// 제작대 제작품 정보
struct st_CraftingTableRecipe
{
	en_GameObjectType CraftingTableType; // 제작대 종류
	wstring CraftingTableName;			 // 제작대 이름	
	vector<CItem*> CraftingTableCompleteItems;	// 제작대 제작품 목록
};

struct st_FieldOfViewInfo
{
	int64 ObjectID;
	int64 SessionID = 0;
	en_GameObjectType ObjectType;

	bool operator<(st_FieldOfViewInfo Right)
	{
		return this->ObjectID < Right.ObjectID;
	}
};

struct st_GameObjectJob
{
	en_GameObjectJobType GameObjectJobType;
	CGameServerMessage* GameObjectJobMessage;
};

struct st_Aggro
{
	CGameObject* AggroTarget;
	float AggroPoint;
};

struct st_MapInfo
{
	int16 MapID;
	wstring MapName;
	int32 MapSectorSize;
	int8 ChannelCount;
};

struct st_OptionItemInfo
{
	en_OptionType OptionType;
	wstring OptionName;
};

struct st_Day
{
	float DayTimeCycle;
	float DayTimeCheck;
	float DayRatio;

	en_DayType DayType;
};