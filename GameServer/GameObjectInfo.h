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
	STATUS_ABNORMAL_ASSASSINATION_BACK_STEP_MASK =			0b11111111111110111111111111111111,				
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
	ITEM_SMALL_CATEGORY_MATERIAL_BRONZE_COIN,
	ITEM_SMALL_CATEGORY_MATERIAL_SLIVER_COIN,
	ITEM_SMALL_CATEGORY_MATERIAL_GOLD_COIN,
	ITEM_SMALL_CATEGORY_MATERIAL_STONE,
	ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG,
	ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK,
	ITEM_SMALL_CATEGORY_MATERIAL_YARN,
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

enum en_EquipmentParts
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
	CLIENT_UI_SPELL_BAR,
	CLIENT_UI_GATHERING_BAR,

	CLIENT_UI_NAME,

	CLIENT_UI_SPEECH_BUBBLE,

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
	COLLISION_POSITION_SKILL_MIDDLE,
	COLLISION_POSITION_SKILL_FRONT
};

enum class en_MenuType : int8
{
	MENU_TYPE_NONE,
	MENU_TYPE_CHARACTER_CHOICE
};


namespace UnityEngine
{
	enum en_UnityKeyCode
	{
		//
		// 요약:
		//     Not assigned (never returned as the result of a keystroke).
		None = 0,
		//
		// 요약:
		//     The backspace key.
		Backspace = 8,
		//
		// 요약:
		//     The tab key.
		Tab = 9,
		//
		// 요약:
		//     The Clear key.
		Clear = 12,
		//
		// 요약:
		//     Return key.
		Return = 13,
		//
		// 요약:
		//     Pause on PC machines.
		Pause = 19,
		//
		// 요약:
		//     Escape key.
		Escape = 27,
		//
		// 요약:
		//     Space key.
		Space = 32,
		//
		// 요약:
		//     Exclamation mark key '!'.
		Exclaim = 33,
		//
		// 요약:
		//     Double quote key '"'.
		DoubleQuote = 34,
		//
		// 요약:
		//     Hash key '#'.
		Hash = 35,
		//
		// 요약:
		//     Dollar sign key '$'.
		Dollar = 36,
		//
		// 요약:
		//     Percent '%' key.
		Percent = 37,
		//
		// 요약:
		//     Ampersand key '&'.
		Ampersand = 38,
		//
		// 요약:
		//     Quote key '.
		Quote = 39,
		//
		// 요약:
		//     Left Parenthesis key '('.
		LeftParen = 40,
		//
		// 요약:
		//     Right Parenthesis key ')'.
		RightParen = 41,
		//
		// 요약:
		//     Asterisk key '*'.
		Asterisk = 42,
		//
		// 요약:
		//     Plus key '+'.
		Plus = 43,
		//
		// 요약:
		//     Comma ',' key.
		Comma = 44,
		//
		// 요약:
		//     Minus '-' key.
		Minus = 45,
		//
		// 요약:
		//     Period '.' key.
		Period = 46,
		//
		// 요약:
		//     Slash '/' key.
		Slash = 47,
		//
		// 요약:
		//     The '0' key on the top of the alphanumeric keyboard.
		Alpha0 = 48,
		//
		// 요약:
		//     The '1' key on the top of the alphanumeric keyboard.
		Alpha1 = 49,
		//
		// 요약:
		//     The '2' key on the top of the alphanumeric keyboard.
		Alpha2 = 50,
		//
		// 요약:
		//     The '3' key on the top of the alphanumeric keyboard.
		Alpha3 = 51,
		//
		// 요약:
		//     The '4' key on the top of the alphanumeric keyboard.
		Alpha4 = 52,
		//
		// 요약:
		//     The '5' key on the top of the alphanumeric keyboard.
		Alpha5 = 53,
		//
		// 요약:
		//     The '6' key on the top of the alphanumeric keyboard.
		Alpha6 = 54,
		//
		// 요약:
		//     The '7' key on the top of the alphanumeric keyboard.
		Alpha7 = 55,
		//
		// 요약:
		//     The '8' key on the top of the alphanumeric keyboard.
		Alpha8 = 56,
		//
		// 요약:
		//     The '9' key on the top of the alphanumeric keyboard.
		Alpha9 = 57,
		//
		// 요약:
		//     Colon ':' key.
		Colon = 58,
		//
		// 요약:
		//     Semicolon ';' key.
		Semicolon = 59,
		//
		// 요약:
		//     Less than '<' key.
		Less = 60,
		//
		// 요약:
		//     Equals '=' key.
		Equals = 61,
		//
		// 요약:
		//     Greater than '>' key.
		Greater = 62,
		//
		// 요약:
		//     Question mark '?' key.
		Question = 63,
		//
		// 요약:
		//     At key '@'.
		At = 64,
		//
		// 요약:
		//     Left square bracket key '['.
		LeftBracket = 91,
		//
		// 요약:
		//     Backslash key '\'.
		Backslash = 92,
		//
		// 요약:
		//     Right square bracket key ']'.
		RightBracket = 93,
		//
		// 요약:
		//     Caret key '^'.
		Caret = 94,
		//
		// 요약:
		//     Underscore '_' key.
		Underscore = 95,
		//
		// 요약:
		//     Back quote key '`'.
		BackQuote = 96,
		//
		// 요약:
		//     'a' key.
		A = 97,
		//
		// 요약:
		//     'b' key.
		B = 98,
		//
		// 요약:
		//     'c' key.
		C = 99,
		//
		// 요약:
		//     'd' key.
		D = 100,
		//
		// 요약:
		//     'e' key.
		E = 101,
		//
		// 요약:
		//     'f' key.
		F = 102,
		//
		// 요약:
		//     'g' key.
		G = 103,
		//
		// 요약:
		//     'h' key.
		H = 104,
		//
		// 요약:
		//     'i' key.
		I = 105,
		//
		// 요약:
		//     'j' key.
		J = 106,
		//
		// 요약:
		//     'k' key.
		K = 107,
		//
		// 요약:
		//     'l' key.
		L = 108,
		//
		// 요약:
		//     'm' key.
		M = 109,
		//
		// 요약:
		//     'n' key.
		N = 110,
		//
		// 요약:
		//     'o' key.
		O = 111,
		//
		// 요약:
		//     'p' key.
		P = 112,
		//
		// 요약:
		//     'q' key.
		Q = 113,
		//
		// 요약:
		//     'r' key.
		R = 114,
		//
		// 요약:
		//     's' key.
		S = 115,
		//
		// 요약:
		//     't' key.
		T = 116,
		//
		// 요약:
		//     'u' key.
		U = 117,
		//
		// 요약:
		//     'v' key.
		V = 118,
		//
		// 요약:
		//     'w' key.
		W = 119,
		//
		// 요약:
		//     'x' key.
		X = 120,
		//
		// 요약:
		//     'y' key.
		Y = 121,
		//
		// 요약:
		//     'z' key.
		Z = 122,
		//
		// 요약:
		//     Left curly bracket key '{'.
		LeftCurlyBracket = 123,
		//
		// 요약:
		//     Pipe '|' key.
		Pipe = 124,
		//
		// 요약:
		//     Right curly bracket key '}'.
		RightCurlyBracket = 125,
		//
		// 요약:
		//     Tilde '~' key.
		Tilde = 126,
		//
		// 요약:
		//     The forward delete key.
		Delete = 127,
		//
		// 요약:
		//     Numeric keypad 0.
		Keypad0 = 256,
		//
		// 요약:
		//     Numeric keypad 1.
		Keypad1 = 257,
		//
		// 요약:
		//     Numeric keypad 2.
		Keypad2 = 258,
		//
		// 요약:
		//     Numeric keypad 3.
		Keypad3 = 259,
		//
		// 요약:
		//     Numeric keypad 4.
		Keypad4 = 260,
		//
		// 요약:
		//     Numeric keypad 5.
		Keypad5 = 261,
		//
		// 요약:
		//     Numeric keypad 6.
		Keypad6 = 262,
		//
		// 요약:
		//     Numeric keypad 7.
		Keypad7 = 263,
		//
		// 요약:
		//     Numeric keypad 8.
		Keypad8 = 264,
		//
		// 요약:
		//     Numeric keypad 9.
		Keypad9 = 265,
		//
		// 요약:
		//     Numeric keypad '.'.
		KeypadPeriod = 266,
		//
		// 요약:
		//     Numeric keypad '/'.
		KeypadDivide = 267,
		//
		// 요약:
		//     Numeric keypad '*'.
		KeypadMultiply = 268,
		//
		// 요약:
		//     Numeric keypad '-'.
		KeypadMinus = 269,
		//
		// 요약:
		//     Numeric keypad '+'.
		KeypadPlus = 270,
		//
		// 요약:
		//     Numeric keypad Enter.
		KeypadEnter = 271,
		//
		// 요약:
		//     Numeric keypad '='.
		KeypadEquals = 272,
		//
		// 요약:
		//     Up arrow key.
		UpArrow = 273,
		//
		// 요약:
		//     Down arrow key.
		DownArrow = 274,
		//
		// 요약:
		//     Right arrow key.
		RightArrow = 275,
		//
		// 요약:
		//     Left arrow key.
		LeftArrow = 276,
		//
		// 요약:
		//     Insert key key.
		Insert = 277,
		//
		// 요약:
		//     Home key.
		Home = 278,
		//
		// 요약:
		//     End key.
		End = 279,
		//
		// 요약:
		//     Page up.
		PageUp = 280,
		//
		// 요약:
		//     Page down.
		PageDown = 281,
		//
		// 요약:
		//     F1 function key.
		F1 = 282,
		//
		// 요약:
		//     F2 function key.
		F2 = 283,
		//
		// 요약:
		//     F3 function key.
		F3 = 284,
		//
		// 요약:
		//     F4 function key.
		F4 = 285,
		//
		// 요약:
		//     F5 function key.
		F5 = 286,
		//
		// 요약:
		//     F6 function key.
		F6 = 287,
		//
		// 요약:
		//     F7 function key.
		F7 = 288,
		//
		// 요약:
		//     F8 function key.
		F8 = 289,
		//
		// 요약:
		//     F9 function key.
		F9 = 290,
		//
		// 요약:
		//     F10 function key.
		F10 = 291,
		//
		// 요약:
		//     F11 function key.
		F11 = 292,
		//
		// 요약:
		//     F12 function key.
		F12 = 293,
		//
		// 요약:
		//     F13 function key.
		F13 = 294,
		//
		// 요약:
		//     F14 function key.
		F14 = 295,
		//
		// 요약:
		//     F15 function key.
		F15 = 296,
		//
		// 요약:
		//     Numlock key.
		Numlock = 300,
		//
		// 요약:
		//     Capslock key.
		CapsLock = 301,
		//
		// 요약:
		//     Scroll lock key.
		ScrollLock = 302,
		//
		// 요약:
		//     Right shift key.
		RightShift = 303,
		//
		// 요약:
		//     Left shift key.
		LeftShift = 304,
		//
		// 요약:
		//     Right Control key.
		RightControl = 305,
		//
		// 요약:
		//     Left Control key.
		LeftControl = 306,
		//
		// 요약:
		//     Right Alt key.
		RightAlt = 307,
		//
		// 요약:
		//     Left Alt key.
		LeftAlt = 308,
		//
		// 요약:
		//     Right Command key.
		RightCommand = 309,
		//
		// 요약:
		//     Right Command key.
		RightApple = 309,
		//
		// 요약:
		//     Left Command key.
		LeftCommand = 310,
		//
		// 요약:
		//     Left Command key.
		LeftApple = 310,
		//
		// 요약:
		//     Left Windows key.
		LeftWindows = 311,
		//
		// 요약:
		//     Right Windows key.
		RightWindows = 312,
		//
		// 요약:
		//     Alt Gr key.
		AltGr = 313,
		//
		// 요약:
		//     Help key.
		Help = 315,
		//
		// 요약:
		//     Print key.
		Print = 316,
		//
		// 요약:
		//     Sys Req key.
		SysReq = 317,
		//
		// 요약:
		//     Break key.
		Break = 318,
		//
		// 요약:
		//     Menu key.
		Menu = 319,
		//
		// 요약:
		//     The Left (or primary) mouse button.
		Mouse0 = 323,
		//
		// 요약:
		//     Right mouse button (or secondary mouse button).
		Mouse1 = 324,
		//
		// 요약:
		//     Middle mouse button (or third button).
		Mouse2 = 325,
		//
		// 요약:
		//     Additional (fourth) mouse button.
		Mouse3 = 326,
		//
		// 요약:
		//     Additional (fifth) mouse button.
		Mouse4 = 327,
		//
		// 요약:
		//     Additional (or sixth) mouse button.
		Mouse5 = 328,
		//
		// 요약:
		//     Additional (or seventh) mouse button.
		Mouse6 = 329,
		//
		// 요약:
		//     Button 0 on any joystick.
		JoystickButton0 = 330,
		//
		// 요약:
		//     Button 1 on any joystick.
		JoystickButton1 = 331,
		//
		// 요약:
		//     Button 2 on any joystick.
		JoystickButton2 = 332,
		//
		// 요약:
		//     Button 3 on any joystick.
		JoystickButton3 = 333,
		//
		// 요약:
		//     Button 4 on any joystick.
		JoystickButton4 = 334,
		//
		// 요약:
		//     Button 5 on any joystick.
		JoystickButton5 = 335,
		//
		// 요약:
		//     Button 6 on any joystick.
		JoystickButton6 = 336,
		//
		// 요약:
		//     Button 7 on any joystick.
		JoystickButton7 = 337,
		//
		// 요약:
		//     Button 8 on any joystick.
		JoystickButton8 = 338,
		//
		// 요약:
		//     Button 9 on any joystick.
		JoystickButton9 = 339,
		//
		// 요약:
		//     Button 10 on any joystick.
		JoystickButton10 = 340,
		//
		// 요약:
		//     Button 11 on any joystick.
		JoystickButton11 = 341,
		//
		// 요약:
		//     Button 12 on any joystick.
		JoystickButton12 = 342,
		//
		// 요약:
		//     Button 13 on any joystick.
		JoystickButton13 = 343,
		//
		// 요약:
		//     Button 14 on any joystick.
		JoystickButton14 = 344,
		//
		// 요약:
		//     Button 15 on any joystick.
		JoystickButton15 = 345,
		//
		// 요약:
		//     Button 16 on any joystick.
		JoystickButton16 = 346,
		//
		// 요약:
		//     Button 17 on any joystick.
		JoystickButton17 = 347,
		//
		// 요약:
		//     Button 18 on any joystick.
		JoystickButton18 = 348,
		//
		// 요약:
		//     Button 19 on any joystick.
		JoystickButton19 = 349,
		//
		// 요약:
		//     Button 0 on first joystick.
		Joystick1Button0 = 350,
		//
		// 요약:
		//     Button 1 on first joystick.
		Joystick1Button1 = 351,
		//
		// 요약:
		//     Button 2 on first joystick.
		Joystick1Button2 = 352,
		//
		// 요약:
		//     Button 3 on first joystick.
		Joystick1Button3 = 353,
		//
		// 요약:
		//     Button 4 on first joystick.
		Joystick1Button4 = 354,
		//
		// 요약:
		//     Button 5 on first joystick.
		Joystick1Button5 = 355,
		//
		// 요약:
		//     Button 6 on first joystick.
		Joystick1Button6 = 356,
		//
		// 요약:
		//     Button 7 on first joystick.
		Joystick1Button7 = 357,
		//
		// 요약:
		//     Button 8 on first joystick.
		Joystick1Button8 = 358,
		//
		// 요약:
		//     Button 9 on first joystick.
		Joystick1Button9 = 359,
		//
		// 요약:
		//     Button 10 on first joystick.
		Joystick1Button10 = 360,
		//
		// 요약:
		//     Button 11 on first joystick.
		Joystick1Button11 = 361,
		//
		// 요약:
		//     Button 12 on first joystick.
		Joystick1Button12 = 362,
		//
		// 요약:
		//     Button 13 on first joystick.
		Joystick1Button13 = 363,
		//
		// 요약:
		//     Button 14 on first joystick.
		Joystick1Button14 = 364,
		//
		// 요약:
		//     Button 15 on first joystick.
		Joystick1Button15 = 365,
		//
		// 요약:
		//     Button 16 on first joystick.
		Joystick1Button16 = 366,
		//
		// 요약:
		//     Button 17 on first joystick.
		Joystick1Button17 = 367,
		//
		// 요약:
		//     Button 18 on first joystick.
		Joystick1Button18 = 368,
		//
		// 요약:
		//     Button 19 on first joystick.
		Joystick1Button19 = 369,
		//
		// 요약:
		//     Button 0 on second joystick.
		Joystick2Button0 = 370,
		//
		// 요약:
		//     Button 1 on second joystick.
		Joystick2Button1 = 371,
		//
		// 요약:
		//     Button 2 on second joystick.
		Joystick2Button2 = 372,
		//
		// 요약:
		//     Button 3 on second joystick.
		Joystick2Button3 = 373,
		//
		// 요약:
		//     Button 4 on second joystick.
		Joystick2Button4 = 374,
		//
		// 요약:
		//     Button 5 on second joystick.
		Joystick2Button5 = 375,
		//
		// 요약:
		//     Button 6 on second joystick.
		Joystick2Button6 = 376,
		//
		// 요약:
		//     Button 7 on second joystick.
		Joystick2Button7 = 377,
		//
		// 요약:
		//     Button 8 on second joystick.
		Joystick2Button8 = 378,
		//
		// 요약:
		//     Button 9 on second joystick.
		Joystick2Button9 = 379,
		//
		// 요약:
		//     Button 10 on second joystick.
		Joystick2Button10 = 380,
		//
		// 요약:
		//     Button 11 on second joystick.
		Joystick2Button11 = 381,
		//
		// 요약:
		//     Button 12 on second joystick.
		Joystick2Button12 = 382,
		//
		// 요약:
		//     Button 13 on second joystick.
		Joystick2Button13 = 383,
		//
		// 요약:
		//     Button 14 on second joystick.
		Joystick2Button14 = 384,
		//
		// 요약:
		//     Button 15 on second joystick.
		Joystick2Button15 = 385,
		//
		// 요약:
		//     Button 16 on second joystick.
		Joystick2Button16 = 386,
		//
		// 요약:
		//     Button 17 on second joystick.
		Joystick2Button17 = 387,
		//
		// 요약:
		//     Button 18 on second joystick.
		Joystick2Button18 = 388,
		//
		// 요약:
		//     Button 19 on second joystick.
		Joystick2Button19 = 389,
		//
		// 요약:
		//     Button 0 on third joystick.
		Joystick3Button0 = 390,
		//
		// 요약:
		//     Button 1 on third joystick.
		Joystick3Button1 = 391,
		//
		// 요약:
		//     Button 2 on third joystick.
		Joystick3Button2 = 392,
		//
		// 요약:
		//     Button 3 on third joystick.
		Joystick3Button3 = 393,
		//
		// 요약:
		//     Button 4 on third joystick.
		Joystick3Button4 = 394,
		//
		// 요약:
		//     Button 5 on third joystick.
		Joystick3Button5 = 395,
		//
		// 요약:
		//     Button 6 on third joystick.
		Joystick3Button6 = 396,
		//
		// 요약:
		//     Button 7 on third joystick.
		Joystick3Button7 = 397,
		//
		// 요약:
		//     Button 8 on third joystick.
		Joystick3Button8 = 398,
		//
		// 요약:
		//     Button 9 on third joystick.
		Joystick3Button9 = 399,
		//
		// 요약:
		//     Button 10 on third joystick.
		Joystick3Button10 = 400,
		//
		// 요약:
		//     Button 11 on third joystick.
		Joystick3Button11 = 401,
		//
		// 요약:
		//     Button 12 on third joystick.
		Joystick3Button12 = 402,
		//
		// 요약:
		//     Button 13 on third joystick.
		Joystick3Button13 = 403,
		//
		// 요약:
		//     Button 14 on third joystick.
		Joystick3Button14 = 404,
		//
		// 요약:
		//     Button 15 on third joystick.
		Joystick3Button15 = 405,
		//
		// 요약:
		//     Button 16 on third joystick.
		Joystick3Button16 = 406,
		//
		// 요약:
		//     Button 17 on third joystick.
		Joystick3Button17 = 407,
		//
		// 요약:
		//     Button 18 on third joystick.
		Joystick3Button18 = 408,
		//
		// 요약:
		//     Button 19 on third joystick.
		Joystick3Button19 = 409,
		//
		// 요약:
		//     Button 0 on forth joystick.
		Joystick4Button0 = 410,
		//
		// 요약:
		//     Button 1 on forth joystick.
		Joystick4Button1 = 411,
		//
		// 요약:
		//     Button 2 on forth joystick.
		Joystick4Button2 = 412,
		//
		// 요약:
		//     Button 3 on forth joystick.
		Joystick4Button3 = 413,
		//
		// 요약:
		//     Button 4 on forth joystick.
		Joystick4Button4 = 414,
		//
		// 요약:
		//     Button 5 on forth joystick.
		Joystick4Button5 = 415,
		//
		// 요약:
		//     Button 6 on forth joystick.
		Joystick4Button6 = 416,
		//
		// 요약:
		//     Button 7 on forth joystick.
		Joystick4Button7 = 417,
		//
		// 요약:
		//     Button 8 on forth joystick.
		Joystick4Button8 = 418,
		//
		// 요약:
		//     Button 9 on forth joystick.
		Joystick4Button9 = 419,
		//
		// 요약:
		//     Button 10 on forth joystick.
		Joystick4Button10 = 420,
		//
		// 요약:
		//     Button 11 on forth joystick.
		Joystick4Button11 = 421,
		//
		// 요약:
		//     Button 12 on forth joystick.
		Joystick4Button12 = 422,
		//
		// 요약:
		//     Button 13 on forth joystick.
		Joystick4Button13 = 423,
		//
		// 요약:
		//     Button 14 on forth joystick.
		Joystick4Button14 = 424,
		//
		// 요약:
		//     Button 15 on forth joystick.
		Joystick4Button15 = 425,
		//
		// 요약:
		//     Button 16 on forth joystick.
		Joystick4Button16 = 426,
		//
		// 요약:
		//     Button 17 on forth joystick.
		Joystick4Button17 = 427,
		//
		// 요약:
		//     Button 18 on forth joystick.
		Joystick4Button18 = 428,
		//
		// 요약:
		//     Button 19 on forth joystick.
		Joystick4Button19 = 429,
		//
		// 요약:
		//     Button 0 on fifth joystick.
		Joystick5Button0 = 430,
		//
		// 요약:
		//     Button 1 on fifth joystick.
		Joystick5Button1 = 431,
		//
		// 요약:
		//     Button 2 on fifth joystick.
		Joystick5Button2 = 432,
		//
		// 요약:
		//     Button 3 on fifth joystick.
		Joystick5Button3 = 433,
		//
		// 요약:
		//     Button 4 on fifth joystick.
		Joystick5Button4 = 434,
		//
		// 요약:
		//     Button 5 on fifth joystick.
		Joystick5Button5 = 435,
		//
		// 요약:
		//     Button 6 on fifth joystick.
		Joystick5Button6 = 436,
		//
		// 요약:
		//     Button 7 on fifth joystick.
		Joystick5Button7 = 437,
		//
		// 요약:
		//     Button 8 on fifth joystick.
		Joystick5Button8 = 438,
		//
		// 요약:
		//     Button 9 on fifth joystick.
		Joystick5Button9 = 439,
		//
		// 요약:
		//     Button 10 on fifth joystick.
		Joystick5Button10 = 440,
		//
		// 요약:
		//     Button 11 on fifth joystick.
		Joystick5Button11 = 441,
		//
		// 요약:
		//     Button 12 on fifth joystick.
		Joystick5Button12 = 442,
		//
		// 요약:
		//     Button 13 on fifth joystick.
		Joystick5Button13 = 443,
		//
		// 요약:
		//     Button 14 on fifth joystick.
		Joystick5Button14 = 444,
		//
		// 요약:
		//     Button 15 on fifth joystick.
		Joystick5Button15 = 445,
		//
		// 요약:
		//     Button 16 on fifth joystick.
		Joystick5Button16 = 446,
		//
		// 요약:
		//     Button 17 on fifth joystick.
		Joystick5Button17 = 447,
		//
		// 요약:
		//     Button 18 on fifth joystick.
		Joystick5Button18 = 448,
		//
		// 요약:
		//     Button 19 on fifth joystick.
		Joystick5Button19 = 449,
		//
		// 요약:
		//     Button 0 on sixth joystick.
		Joystick6Button0 = 450,
		//
		// 요약:
		//     Button 1 on sixth joystick.
		Joystick6Button1 = 451,
		//
		// 요약:
		//     Button 2 on sixth joystick.
		Joystick6Button2 = 452,
		//
		// 요약:
		//     Button 3 on sixth joystick.
		Joystick6Button3 = 453,
		//
		// 요약:
		//     Button 4 on sixth joystick.
		Joystick6Button4 = 454,
		//
		// 요약:
		//     Button 5 on sixth joystick.
		Joystick6Button5 = 455,
		//
		// 요약:
		//     Button 6 on sixth joystick.
		Joystick6Button6 = 456,
		//
		// 요약:
		//     Button 7 on sixth joystick.
		Joystick6Button7 = 457,
		//
		// 요약:
		//     Button 8 on sixth joystick.
		Joystick6Button8 = 458,
		//
		// 요약:
		//     Button 9 on sixth joystick.
		Joystick6Button9 = 459,
		//
		// 요약:
		//     Button 10 on sixth joystick.
		Joystick6Button10 = 460,
		//
		// 요약:
		//     Button 11 on sixth joystick.
		Joystick6Button11 = 461,
		//
		// 요약:
		//     Button 12 on sixth joystick.
		Joystick6Button12 = 462,
		//
		// 요약:
		//     Button 13 on sixth joystick.
		Joystick6Button13 = 463,
		//
		// 요약:
		//     Button 14 on sixth joystick.
		Joystick6Button14 = 464,
		//
		// 요약:
		//     Button 15 on sixth joystick.
		Joystick6Button15 = 465,
		//
		// 요약:
		//     Button 16 on sixth joystick.
		Joystick6Button16 = 466,
		//
		// 요약:
		//     Button 17 on sixth joystick.
		Joystick6Button17 = 467,
		//
		// 요약:
		//     Button 18 on sixth joystick.
		Joystick6Button18 = 468,
		//
		// 요약:
		//     Button 19 on sixth joystick.
		Joystick6Button19 = 469,
		//
		// 요약:
		//     Button 0 on seventh joystick.
		Joystick7Button0 = 470,
		//
		// 요약:
		//     Button 1 on seventh joystick.
		Joystick7Button1 = 471,
		//
		// 요약:
		//     Button 2 on seventh joystick.
		Joystick7Button2 = 472,
		//
		// 요약:
		//     Button 3 on seventh joystick.
		Joystick7Button3 = 473,
		//
		// 요약:
		//     Button 4 on seventh joystick.
		Joystick7Button4 = 474,
		//
		// 요약:
		//     Button 5 on seventh joystick.
		Joystick7Button5 = 475,
		//
		// 요약:
		//     Button 6 on seventh joystick.
		Joystick7Button6 = 476,
		//
		// 요약:
		//     Button 7 on seventh joystick.
		Joystick7Button7 = 477,
		//
		// 요약:
		//     Button 8 on seventh joystick.
		Joystick7Button8 = 478,
		//
		// 요약:
		//     Button 9 on seventh joystick.
		Joystick7Button9 = 479,
		//
		// 요약:
		//     Button 10 on seventh joystick.
		Joystick7Button10 = 480,
		//
		// 요약:
		//     Button 11 on seventh joystick.
		Joystick7Button11 = 481,
		//
		// 요약:
		//     Button 12 on seventh joystick.
		Joystick7Button12 = 482,
		//
		// 요약:
		//     Button 13 on seventh joystick.
		Joystick7Button13 = 483,
		//
		// 요약:
		//     Button 14 on seventh joystick.
		Joystick7Button14 = 484,
		//
		// 요약:
		//     Button 15 on seventh joystick.
		Joystick7Button15 = 485,
		//
		// 요약:
		//     Button 16 on seventh joystick.
		Joystick7Button16 = 486,
		//
		// 요약:
		//     Button 17 on seventh joystick.
		Joystick7Button17 = 487,
		//
		// 요약:
		//     Button 18 on seventh joystick.
		Joystick7Button18 = 488,
		//
		// 요약:
		//     Button 19 on seventh joystick.
		Joystick7Button19 = 489,
		//
		// 요약:
		//     Button 0 on eighth joystick.
		Joystick8Button0 = 490,
		//
		// 요약:
		//     Button 1 on eighth joystick.
		Joystick8Button1 = 491,
		//
		// 요약:
		//     Button 2 on eighth joystick.
		Joystick8Button2 = 492,
		//
		// 요약:
		//     Button 3 on eighth joystick.
		Joystick8Button3 = 493,
		//
		// 요약:
		//     Button 4 on eighth joystick.
		Joystick8Button4 = 494,
		//
		// 요약:
		//     Button 5 on eighth joystick.
		Joystick8Button5 = 495,
		//
		// 요약:
		//     Button 6 on eighth joystick.
		Joystick8Button6 = 496,
		//
		// 요약:
		//     Button 7 on eighth joystick.
		Joystick8Button7 = 497,
		//
		// 요약:
		//     Button 8 on eighth joystick.
		Joystick8Button8 = 498,
		//
		// 요약:
		//     Button 9 on eighth joystick.
		Joystick8Button9 = 499,
		//
		// 요약:
		//     Button 10 on eighth joystick.
		Joystick8Button10 = 500,
		//
		// 요약:
		//     Button 11 on eighth joystick.
		Joystick8Button11 = 501,
		//
		// 요약:
		//     Button 12 on eighth joystick.
		Joystick8Button12 = 502,
		//
		// 요약:
		//     Button 13 on eighth joystick.
		Joystick8Button13 = 503,
		//
		// 요약:
		//     Button 14 on eighth joystick.
		Joystick8Button14 = 504,
		//
		// 요약:
		//     Button 15 on eighth joystick.
		Joystick8Button15 = 505,
		//
		// 요약:
		//     Button 16 on eighth joystick.
		Joystick8Button16 = 506,
		//
		// 요약:
		//     Button 17 on eighth joystick.
		Joystick8Button17 = 507,
		//
		// 요약:
		//     Button 18 on eighth joystick.
		Joystick8Button18 = 508,
		//
		// 요약:
		//     Button 19 on eighth joystick.
		Joystick8Button19 = 509
	};
}

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
	int32 Level;
	int32 Str;
	int32 Dex;
	int32 Int;
	int32 Luck;
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
	int16 ItemWidth;			     		  // 아이템 너비
	int16 ItemHeight;						  // 아이템 높이	
	float ItemCollisionX;					  // 장비 아이템 감지 박스 X 크기
	float ItemCollisionY;					  // 장비 아이템 감지 박스 Y 크기
	int16 ItemTileGridPositionX;			  // 인벤토리 위치 X
	int16 ItemTileGridPositionY;			  // 인벤토리 위치 Y
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
	int16 QuickSlotKey;					 // 퀵슬롯에 연동된 키값
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