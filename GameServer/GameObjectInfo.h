#pragma once
#include <xmmintrin.h>

class CGameObject;
class CGameServerMessage;
class CSkill;
class CItem;

enum class en_GameObjectType : int16
{
	NORMAL,

	OBJECT_PLAYER,
	OBJECT_WARRIOR_PLAYER,
	OBJECT_SHAMAN_PLAYER,
	OBJECT_TAIOIST_PLAYER,
	OBJECT_THIEF_PLAYER,
	OBJECT_ARCHER_PLAYER,

	OBJECT_MONSTER,
	OBJECT_SLIME,
	OBJECT_BEAR,

	OBJECT_ENVIRONMENT,
	OBJECT_STONE,
	OBJECT_TREE,

	OBJECT_ARCHITECTURE,
	OBJECT_ARCHITECTURE_CRAFTING_TABLE,
	OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE,
	OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL,	

	OBJECT_TILE,

	OBJECT_CROP,
	OBJECT_CROP_POTATO,

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
	OBJECT_ITEM_MATERIAL_CHAR_COAL,
	OBJECT_ITEM_MATERIAL_COPPER_NUGGET,
	OBJECT_ITEM_MATERIAL_COPPER_INGOT,
	OBJECT_ITEM_MATERIAL_IRON_NUGGET,
	OBJECT_ITEM_MATERIAL_IRON_INGOT,	

	OBJECT_ITEM_CROP_SEED_POTATO,
	OBJECT_ITEM_CROP_FRUIT_POTATO,

	OBJECT_PLAYER_DUMMY = 32000
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
	STOP,
	RETURN_SPAWN_POSITION,
	ATTACK,
	SPELL,
	GATHERING,
	CRAFTING,
	READY_DEAD,
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
	ITEM_LARGE_CATEGORY_FOOD,
	ITEM_LARGE_CATEGORY_POTION,
	ITEM_LARGE_CATEGORY_SKILLBOOK,
	ITEM_LARGE_CATEGORY_MATERIAL,
	ITEM_LARGE_CATEGORY_CROP
};

enum class en_MediumItemCategory : int8
{
	ITEM_MEDIUM_CATEGORY_NONE = 0,
	ITEM_MEDIUM_CATEGORY_CRAFTING_TABLE,
	ITEM_MEDIUM_CATEGORY_SWORD,
	ITEM_MEDIUM_CATEGORY_HAT,
	ITEM_MEDIUM_CATEGORY_WEAR,
	ITEM_MEDIUM_CATEGORY_GLOVE,
	ITEM_MEDIUM_CATEGORY_BOOT,
	ITEM_MEDIUM_CATEGORY_HEAL,
	ITEM_MEDIUM_CATEGORY_MANA,
	ITEM_MEDIUM_CATEGORY_CROP_SEED,
	ITEM_MEDIUM_CATEGORY_CROP_FRUIT
};

enum class en_SmallItemCategory : int16
{
	ITEM_SMALL_CATEGORY_NONE = 0,

	ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD = 1,

	ITEM_SMALL_CATEGORY_ARMOR_WEAR_WOOD = 100,
	ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER,
	ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER,

	ITEM_SMALL_CATEGORY_POTION_HEAL_SMALL = 200,
	ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_FIERCE_ATTACK = 300,
	ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CONVERSION_ATTACK,
	ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_SHAEHONE_ATTACK,
	ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHOHONE_ATTACK,
	ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_SMASH_WAVE_ATTACK,
	ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHARGE_POSE,
	ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_FLAME_HARPOON,
	ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_HELL_FIRE,
	ITEM_SMALL_CATEOGRY_SKILLBOOK_SHAMAN_HEALING_LIGHT,
	ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_HEALING_WIND,
	ITEM_SMALL_CATEGORY_SKILLBOOK_SHOCK_RELEASE,

	ITEM_SMALL_CATEGORY_MATERIAL_LEATHER = 2000,
	ITEM_SMALL_CATEGORY_MATERIAL_SLIMEGEL,
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

	ITEM_SMALL_CATEGORY_CRAFTING_TABLE_FURANCE = 5000,
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
	SKILL_MEDIUM_CATEGORY_PUBLIC_ATTACK,
	SKILL_MEDIUM_CATEGORY_PUBLIC_TACTIC,
	SKILL_MEDIUM_CATEGORY_PUBLIC_HEAL,
	SKILL_MEDIUM_CATEGORY_PUBLIC_BUF,

	SKILL_MEDIUM_CATEGORY_WARRIOR_ATTACK,
	SKILL_MEDIUM_CATEGORY_WARRIOR_TACTIC,
	SKILL_MEDIUM_CATEGORY_WARRIOR_HEAL,
	SKILL_MEDIUM_CATEGORY_WARRIOR_BUF,

	SKILL_MEDIUM_CATEGORY_SHMAN_ATTACK,
	SKILL_MEDIUM_CATEGORY_SHMAN_TACTIC,
	SKILL_MEDIUM_CATEGORY_SHMAN_HEAL,
	SKILL_MEDIUM_CATEGORY_SHMAN_BUF,

	SKILL_MEDIUM_CATEGORY_TAOIST_ATTACK,
	SKILL_MEDIUM_CATEGORY_TAOIST_TACTIC,
	SKILL_MEDIUM_CATEGORY_TAOIST_HEAL,
	SKILL_MEDIUM_CATEGORY_TAOIST_BUF,

	SKILL_MEDIUM_CATEGORY_THIEF_ATTACK,
	SKILL_MEDIUM_CATEGORY_THIEF_TACTIC,
	SKILL_MEDIUM_CATEGORY_THIEF_HEAL,
	SKILL_MEDIUM_CATEGORY_THIEF_BUF,

	SKILL_MEDIUM_CATEGORY_ARCHER_ATTACK,
	SKILL_MEDIUM_CATEGORY_ARCHER_TACTIC,
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
	SKILL_SHAMAN_BACK_TELEPORT,

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

enum class en_SkillCategory : int8
{
	QUICK_SLOT_SKILL_COOLTIME,
	STATUS_ABNORMAL_SKILL,
	COMBO_SKILL
};

enum class en_SkillKinds : int8
{
	SKILL_KIND_NONE,
	MELEE_SKILL,
	MAGIC_SKILL
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
	EFFECT_LIGHTNING,
	EFFECT_BACK_TELEPORT,
	EFFECT_HEALING_LIGHT_TARGET,
	EFFECT_HEALING_WIND_TARGET,
	EFFECT_HELAING_MYSELF,
	EFFECT_OBJECT_SPAWN,
	EFFECT_DEBUF_ROOT,
	EFFECT_DEBUF_STUN
};

enum class en_CommonErrorType : int16
{
	ERROR_STATUS_ABNORMAL_MOVE,
	ERROR_STATUS_ABNORMAL_MELEE,
	ERROR_STATUS_ABNORMAL_MAGIC
};

enum class en_PersonalMessageType : int8
{
	PERSONAL_MESSAGE_NONE,

	PERSONAL_MESSAGE_STATUS_ABNORMAL,
	PERSONAL_MESSAGE_STATUS_ABNORMAL_WARRIOR_CHOHONE,
	PERSONAL_MESSAGE_STATUS_ABNORMAL_WARRIOR_SHAEHONE,
	PERSONAL_MESSAGE_STATUS_ABNORMAL_SHAMAN_ROOT,
	PERSONAL_MESSAGE_STATUS_ABNORMAL_SHAMAN_ICE_CHAIN,
	PERSONAL_MESSAGE_STATUS_ABNORMAL_SHAMAN_ICE_WAVE,
	PERSONAL_MESSAGE_STATUS_ABNORMAL_SHAMAN_LIGHTNING_STRIKE,
	PERSONAL_MESSAGE_STATUS_ABNORMAL_TAIOIST_ROOT,

	PERSONAL_MESSAGE_SKILL_COOLTIME,
	PERSONAL_MESSAGE_NON_SELECT_OBJECT,
	PERSONAL_MESSAGE_HEAL_NON_SELECT_OBJECT,
	PERSONAL_MESSAGE_PLACE_BLOCK,
	PERSONAL_MESSAGE_PLACE_DISTANCE,
	PERSONAL_MESSAGE_MYSELF_TARGET,
		
	PERSONAL_MESSAGE_DIR_DIFFERENT,
	PERSONAL_MESSAGE_GATHERING_DISTANCE,

	PERSONAL_MEESAGE_CRAFTING_TABLE_OVERLAP_SELECT,
	PERSONAL_MESSAGE_CRAFTING_TABLE_OVERLAP_CRAFTING_START,
	PERSONAL_MESSAGE_CRAFTING_TABLE_MATERIAL_COUNT_NOT_ENOUGH,
	PERSOANL_MESSAGE_CRAFTING_TABLE_MATERIAL_WRONG_ITEM_ADD,

	PERSONAL_MESSAGE_LOGIN_ACCOUNT_NOT_EXIST,
	PERSONAL_MESSAGE_LOGIN_ACCOUNT_OVERLAP,
	PERSONAL_MESSAGE_LOGIN_ACCOUNT_DB_WORKING,
	PERSONAL_MESSAGE_LOGIN_ACCOUNT_DIFFERENT_PASSWORD
};

enum class en_ConsumableType : int16
{
	NONE,
	POTION,
	SKILL_BOOK
};

enum class en_MapObjectInfo : int8
{
	TILE_MAP_NONE = 0,
	TILE_MAP_WALL,
	TILE_MAP_TREE,
	TILE_MAP_STONE,
	TILE_MAP_SLIME,
	TILE_MAP_BEAR,
	TILE_MAP_FURNACE,
	TILE_MAP_SAMILL,
	TILE_MAP_POTATO	
};

enum class en_MapTileInfo : int8
{	
	MAP_TILE_USER_FREE = 0,
	MAP_TILE_SYSTEM_ALLOC,
	MAP_TILE_USER_ALLOC
};

enum class en_GameObjectJobType : int16
{
	GAMEOBJECT_JOB_TYPE_SHOCK_RELEASE,
	GAMEOBJECT_JOB_TYPE_BACK_TELEPORT,		
	GAMEOBJECT_JOB_TYPE_COMBO_ATTACK_CREATE,
	GAMEOBJECT_JOB_TYPE_COMBO_ATTACK_OFF,
	GAMEOBJECT_JOB_TYPE_SPELL_START,	
	GAMEOBJECT_JOB_TYPE_SPELL_CANCEL,
	GAMEOBJECT_JOB_TYPE_GATHERING_START,
	GAMEOBJECT_JOB_TYPE_GATHERING_CANCEL,	
	GAMEOBJECT_JOB_AGGRO_LIST_INSERT_OR_UPDATE,
	GAMEOBJECT_JOB_AGGRO_LIST_REMOVE,
	GAMEOBJECT_JOB_DAMAGE,
	GAMEOJBECT_JOB_HEAL,
	GAMEOBJECT_JOB_PLAYER_ENTER_CHANNEL,
	GAMEOBJECT_JOB_OBJECT_ENTER_CHANNEL,
	GAMEOBJECT_JOB_LEAVE_CHANNEL,
	GAMEOBJECT_JOB_PLAYER_LEAVE_CHANNEL,
	GAMEOBJECT_JOB_FULL_RECOVERY,
	GAMEOBJECT_JOB_ITEM_DROP,
	GAMEOBJECT_JOB_CRAFTING_TABLE_SELECT,
	GAMEOJBECT_JOB_CRAFTING_TABLE_NON_SELECT,
	GAMEOBJECT_JOB_CRAFTING_TABLE_ITEM_ADD,
	GAMEOBJECT_JOB_CRAFTING_TABLE_MATERIAL_ITEM_SUBTRACT,
	GAMEOBJECT_JOB_CRAFTING_TABLE_COMPLETE_ITEM_SUBTRACT,
	GAMEOBJECT_JOB_CRAFTING_TABLE_CRAFTING_START,
	GAMEOBJECT_JOB_CRAFTING_TABLE_CRAFTING_STOP
};

enum class en_MonsterAggroType : int8
{
	MONSTER_AGGRO_FIRST_TARGET,
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

// �� Ÿ�Ͽ� ���� �� �� �ִ� �������� �ִ� ���� ����
enum class en_MapItemInfo : int8
{
	MAP_ITEM_COUNT_MAX = 20
};

enum class en_OptionType : int8
{
	OPTION_TYPE_NONE = 0,
	OPTION_TYPE_TILE_BUY
};

namespace UnityEngine
{
	enum en_UnityKeyCode
	{
		//
		// ���:
		//     Not assigned (never returned as the result of a keystroke).
		None = 0,
		//
		// ���:
		//     The backspace key.
		Backspace = 8,
		//
		// ���:
		//     The tab key.
		Tab = 9,
		//
		// ���:
		//     The Clear key.
		Clear = 12,
		//
		// ���:
		//     Return key.
		Return = 13,
		//
		// ���:
		//     Pause on PC machines.
		Pause = 19,
		//
		// ���:
		//     Escape key.
		Escape = 27,
		//
		// ���:
		//     Space key.
		Space = 32,
		//
		// ���:
		//     Exclamation mark key '!'.
		Exclaim = 33,
		//
		// ���:
		//     Double quote key '"'.
		DoubleQuote = 34,
		//
		// ���:
		//     Hash key '#'.
		Hash = 35,
		//
		// ���:
		//     Dollar sign key '$'.
		Dollar = 36,
		//
		// ���:
		//     Percent '%' key.
		Percent = 37,
		//
		// ���:
		//     Ampersand key '&'.
		Ampersand = 38,
		//
		// ���:
		//     Quote key '.
		Quote = 39,
		//
		// ���:
		//     Left Parenthesis key '('.
		LeftParen = 40,
		//
		// ���:
		//     Right Parenthesis key ')'.
		RightParen = 41,
		//
		// ���:
		//     Asterisk key '*'.
		Asterisk = 42,
		//
		// ���:
		//     Plus key '+'.
		Plus = 43,
		//
		// ���:
		//     Comma ',' key.
		Comma = 44,
		//
		// ���:
		//     Minus '-' key.
		Minus = 45,
		//
		// ���:
		//     Period '.' key.
		Period = 46,
		//
		// ���:
		//     Slash '/' key.
		Slash = 47,
		//
		// ���:
		//     The '0' key on the top of the alphanumeric keyboard.
		Alpha0 = 48,
		//
		// ���:
		//     The '1' key on the top of the alphanumeric keyboard.
		Alpha1 = 49,
		//
		// ���:
		//     The '2' key on the top of the alphanumeric keyboard.
		Alpha2 = 50,
		//
		// ���:
		//     The '3' key on the top of the alphanumeric keyboard.
		Alpha3 = 51,
		//
		// ���:
		//     The '4' key on the top of the alphanumeric keyboard.
		Alpha4 = 52,
		//
		// ���:
		//     The '5' key on the top of the alphanumeric keyboard.
		Alpha5 = 53,
		//
		// ���:
		//     The '6' key on the top of the alphanumeric keyboard.
		Alpha6 = 54,
		//
		// ���:
		//     The '7' key on the top of the alphanumeric keyboard.
		Alpha7 = 55,
		//
		// ���:
		//     The '8' key on the top of the alphanumeric keyboard.
		Alpha8 = 56,
		//
		// ���:
		//     The '9' key on the top of the alphanumeric keyboard.
		Alpha9 = 57,
		//
		// ���:
		//     Colon ':' key.
		Colon = 58,
		//
		// ���:
		//     Semicolon ';' key.
		Semicolon = 59,
		//
		// ���:
		//     Less than '<' key.
		Less = 60,
		//
		// ���:
		//     Equals '=' key.
		Equals = 61,
		//
		// ���:
		//     Greater than '>' key.
		Greater = 62,
		//
		// ���:
		//     Question mark '?' key.
		Question = 63,
		//
		// ���:
		//     At key '@'.
		At = 64,
		//
		// ���:
		//     Left square bracket key '['.
		LeftBracket = 91,
		//
		// ���:
		//     Backslash key '\'.
		Backslash = 92,
		//
		// ���:
		//     Right square bracket key ']'.
		RightBracket = 93,
		//
		// ���:
		//     Caret key '^'.
		Caret = 94,
		//
		// ���:
		//     Underscore '_' key.
		Underscore = 95,
		//
		// ���:
		//     Back quote key '`'.
		BackQuote = 96,
		//
		// ���:
		//     'a' key.
		A = 97,
		//
		// ���:
		//     'b' key.
		B = 98,
		//
		// ���:
		//     'c' key.
		C = 99,
		//
		// ���:
		//     'd' key.
		D = 100,
		//
		// ���:
		//     'e' key.
		E = 101,
		//
		// ���:
		//     'f' key.
		F = 102,
		//
		// ���:
		//     'g' key.
		G = 103,
		//
		// ���:
		//     'h' key.
		H = 104,
		//
		// ���:
		//     'i' key.
		I = 105,
		//
		// ���:
		//     'j' key.
		J = 106,
		//
		// ���:
		//     'k' key.
		K = 107,
		//
		// ���:
		//     'l' key.
		L = 108,
		//
		// ���:
		//     'm' key.
		M = 109,
		//
		// ���:
		//     'n' key.
		N = 110,
		//
		// ���:
		//     'o' key.
		O = 111,
		//
		// ���:
		//     'p' key.
		P = 112,
		//
		// ���:
		//     'q' key.
		Q = 113,
		//
		// ���:
		//     'r' key.
		R = 114,
		//
		// ���:
		//     's' key.
		S = 115,
		//
		// ���:
		//     't' key.
		T = 116,
		//
		// ���:
		//     'u' key.
		U = 117,
		//
		// ���:
		//     'v' key.
		V = 118,
		//
		// ���:
		//     'w' key.
		W = 119,
		//
		// ���:
		//     'x' key.
		X = 120,
		//
		// ���:
		//     'y' key.
		Y = 121,
		//
		// ���:
		//     'z' key.
		Z = 122,
		//
		// ���:
		//     Left curly bracket key '{'.
		LeftCurlyBracket = 123,
		//
		// ���:
		//     Pipe '|' key.
		Pipe = 124,
		//
		// ���:
		//     Right curly bracket key '}'.
		RightCurlyBracket = 125,
		//
		// ���:
		//     Tilde '~' key.
		Tilde = 126,
		//
		// ���:
		//     The forward delete key.
		Delete = 127,
		//
		// ���:
		//     Numeric keypad 0.
		Keypad0 = 256,
		//
		// ���:
		//     Numeric keypad 1.
		Keypad1 = 257,
		//
		// ���:
		//     Numeric keypad 2.
		Keypad2 = 258,
		//
		// ���:
		//     Numeric keypad 3.
		Keypad3 = 259,
		//
		// ���:
		//     Numeric keypad 4.
		Keypad4 = 260,
		//
		// ���:
		//     Numeric keypad 5.
		Keypad5 = 261,
		//
		// ���:
		//     Numeric keypad 6.
		Keypad6 = 262,
		//
		// ���:
		//     Numeric keypad 7.
		Keypad7 = 263,
		//
		// ���:
		//     Numeric keypad 8.
		Keypad8 = 264,
		//
		// ���:
		//     Numeric keypad 9.
		Keypad9 = 265,
		//
		// ���:
		//     Numeric keypad '.'.
		KeypadPeriod = 266,
		//
		// ���:
		//     Numeric keypad '/'.
		KeypadDivide = 267,
		//
		// ���:
		//     Numeric keypad '*'.
		KeypadMultiply = 268,
		//
		// ���:
		//     Numeric keypad '-'.
		KeypadMinus = 269,
		//
		// ���:
		//     Numeric keypad '+'.
		KeypadPlus = 270,
		//
		// ���:
		//     Numeric keypad Enter.
		KeypadEnter = 271,
		//
		// ���:
		//     Numeric keypad '='.
		KeypadEquals = 272,
		//
		// ���:
		//     Up arrow key.
		UpArrow = 273,
		//
		// ���:
		//     Down arrow key.
		DownArrow = 274,
		//
		// ���:
		//     Right arrow key.
		RightArrow = 275,
		//
		// ���:
		//     Left arrow key.
		LeftArrow = 276,
		//
		// ���:
		//     Insert key key.
		Insert = 277,
		//
		// ���:
		//     Home key.
		Home = 278,
		//
		// ���:
		//     End key.
		End = 279,
		//
		// ���:
		//     Page up.
		PageUp = 280,
		//
		// ���:
		//     Page down.
		PageDown = 281,
		//
		// ���:
		//     F1 function key.
		F1 = 282,
		//
		// ���:
		//     F2 function key.
		F2 = 283,
		//
		// ���:
		//     F3 function key.
		F3 = 284,
		//
		// ���:
		//     F4 function key.
		F4 = 285,
		//
		// ���:
		//     F5 function key.
		F5 = 286,
		//
		// ���:
		//     F6 function key.
		F6 = 287,
		//
		// ���:
		//     F7 function key.
		F7 = 288,
		//
		// ���:
		//     F8 function key.
		F8 = 289,
		//
		// ���:
		//     F9 function key.
		F9 = 290,
		//
		// ���:
		//     F10 function key.
		F10 = 291,
		//
		// ���:
		//     F11 function key.
		F11 = 292,
		//
		// ���:
		//     F12 function key.
		F12 = 293,
		//
		// ���:
		//     F13 function key.
		F13 = 294,
		//
		// ���:
		//     F14 function key.
		F14 = 295,
		//
		// ���:
		//     F15 function key.
		F15 = 296,
		//
		// ���:
		//     Numlock key.
		Numlock = 300,
		//
		// ���:
		//     Capslock key.
		CapsLock = 301,
		//
		// ���:
		//     Scroll lock key.
		ScrollLock = 302,
		//
		// ���:
		//     Right shift key.
		RightShift = 303,
		//
		// ���:
		//     Left shift key.
		LeftShift = 304,
		//
		// ���:
		//     Right Control key.
		RightControl = 305,
		//
		// ���:
		//     Left Control key.
		LeftControl = 306,
		//
		// ���:
		//     Right Alt key.
		RightAlt = 307,
		//
		// ���:
		//     Left Alt key.
		LeftAlt = 308,
		//
		// ���:
		//     Right Command key.
		RightCommand = 309,
		//
		// ���:
		//     Right Command key.
		RightApple = 309,
		//
		// ���:
		//     Left Command key.
		LeftCommand = 310,
		//
		// ���:
		//     Left Command key.
		LeftApple = 310,
		//
		// ���:
		//     Left Windows key.
		LeftWindows = 311,
		//
		// ���:
		//     Right Windows key.
		RightWindows = 312,
		//
		// ���:
		//     Alt Gr key.
		AltGr = 313,
		//
		// ���:
		//     Help key.
		Help = 315,
		//
		// ���:
		//     Print key.
		Print = 316,
		//
		// ���:
		//     Sys Req key.
		SysReq = 317,
		//
		// ���:
		//     Break key.
		Break = 318,
		//
		// ���:
		//     Menu key.
		Menu = 319,
		//
		// ���:
		//     The Left (or primary) mouse button.
		Mouse0 = 323,
		//
		// ���:
		//     Right mouse button (or secondary mouse button).
		Mouse1 = 324,
		//
		// ���:
		//     Middle mouse button (or third button).
		Mouse2 = 325,
		//
		// ���:
		//     Additional (fourth) mouse button.
		Mouse3 = 326,
		//
		// ���:
		//     Additional (fifth) mouse button.
		Mouse4 = 327,
		//
		// ���:
		//     Additional (or sixth) mouse button.
		Mouse5 = 328,
		//
		// ���:
		//     Additional (or seventh) mouse button.
		Mouse6 = 329,
		//
		// ���:
		//     Button 0 on any joystick.
		JoystickButton0 = 330,
		//
		// ���:
		//     Button 1 on any joystick.
		JoystickButton1 = 331,
		//
		// ���:
		//     Button 2 on any joystick.
		JoystickButton2 = 332,
		//
		// ���:
		//     Button 3 on any joystick.
		JoystickButton3 = 333,
		//
		// ���:
		//     Button 4 on any joystick.
		JoystickButton4 = 334,
		//
		// ���:
		//     Button 5 on any joystick.
		JoystickButton5 = 335,
		//
		// ���:
		//     Button 6 on any joystick.
		JoystickButton6 = 336,
		//
		// ���:
		//     Button 7 on any joystick.
		JoystickButton7 = 337,
		//
		// ���:
		//     Button 8 on any joystick.
		JoystickButton8 = 338,
		//
		// ���:
		//     Button 9 on any joystick.
		JoystickButton9 = 339,
		//
		// ���:
		//     Button 10 on any joystick.
		JoystickButton10 = 340,
		//
		// ���:
		//     Button 11 on any joystick.
		JoystickButton11 = 341,
		//
		// ���:
		//     Button 12 on any joystick.
		JoystickButton12 = 342,
		//
		// ���:
		//     Button 13 on any joystick.
		JoystickButton13 = 343,
		//
		// ���:
		//     Button 14 on any joystick.
		JoystickButton14 = 344,
		//
		// ���:
		//     Button 15 on any joystick.
		JoystickButton15 = 345,
		//
		// ���:
		//     Button 16 on any joystick.
		JoystickButton16 = 346,
		//
		// ���:
		//     Button 17 on any joystick.
		JoystickButton17 = 347,
		//
		// ���:
		//     Button 18 on any joystick.
		JoystickButton18 = 348,
		//
		// ���:
		//     Button 19 on any joystick.
		JoystickButton19 = 349,
		//
		// ���:
		//     Button 0 on first joystick.
		Joystick1Button0 = 350,
		//
		// ���:
		//     Button 1 on first joystick.
		Joystick1Button1 = 351,
		//
		// ���:
		//     Button 2 on first joystick.
		Joystick1Button2 = 352,
		//
		// ���:
		//     Button 3 on first joystick.
		Joystick1Button3 = 353,
		//
		// ���:
		//     Button 4 on first joystick.
		Joystick1Button4 = 354,
		//
		// ���:
		//     Button 5 on first joystick.
		Joystick1Button5 = 355,
		//
		// ���:
		//     Button 6 on first joystick.
		Joystick1Button6 = 356,
		//
		// ���:
		//     Button 7 on first joystick.
		Joystick1Button7 = 357,
		//
		// ���:
		//     Button 8 on first joystick.
		Joystick1Button8 = 358,
		//
		// ���:
		//     Button 9 on first joystick.
		Joystick1Button9 = 359,
		//
		// ���:
		//     Button 10 on first joystick.
		Joystick1Button10 = 360,
		//
		// ���:
		//     Button 11 on first joystick.
		Joystick1Button11 = 361,
		//
		// ���:
		//     Button 12 on first joystick.
		Joystick1Button12 = 362,
		//
		// ���:
		//     Button 13 on first joystick.
		Joystick1Button13 = 363,
		//
		// ���:
		//     Button 14 on first joystick.
		Joystick1Button14 = 364,
		//
		// ���:
		//     Button 15 on first joystick.
		Joystick1Button15 = 365,
		//
		// ���:
		//     Button 16 on first joystick.
		Joystick1Button16 = 366,
		//
		// ���:
		//     Button 17 on first joystick.
		Joystick1Button17 = 367,
		//
		// ���:
		//     Button 18 on first joystick.
		Joystick1Button18 = 368,
		//
		// ���:
		//     Button 19 on first joystick.
		Joystick1Button19 = 369,
		//
		// ���:
		//     Button 0 on second joystick.
		Joystick2Button0 = 370,
		//
		// ���:
		//     Button 1 on second joystick.
		Joystick2Button1 = 371,
		//
		// ���:
		//     Button 2 on second joystick.
		Joystick2Button2 = 372,
		//
		// ���:
		//     Button 3 on second joystick.
		Joystick2Button3 = 373,
		//
		// ���:
		//     Button 4 on second joystick.
		Joystick2Button4 = 374,
		//
		// ���:
		//     Button 5 on second joystick.
		Joystick2Button5 = 375,
		//
		// ���:
		//     Button 6 on second joystick.
		Joystick2Button6 = 376,
		//
		// ���:
		//     Button 7 on second joystick.
		Joystick2Button7 = 377,
		//
		// ���:
		//     Button 8 on second joystick.
		Joystick2Button8 = 378,
		//
		// ���:
		//     Button 9 on second joystick.
		Joystick2Button9 = 379,
		//
		// ���:
		//     Button 10 on second joystick.
		Joystick2Button10 = 380,
		//
		// ���:
		//     Button 11 on second joystick.
		Joystick2Button11 = 381,
		//
		// ���:
		//     Button 12 on second joystick.
		Joystick2Button12 = 382,
		//
		// ���:
		//     Button 13 on second joystick.
		Joystick2Button13 = 383,
		//
		// ���:
		//     Button 14 on second joystick.
		Joystick2Button14 = 384,
		//
		// ���:
		//     Button 15 on second joystick.
		Joystick2Button15 = 385,
		//
		// ���:
		//     Button 16 on second joystick.
		Joystick2Button16 = 386,
		//
		// ���:
		//     Button 17 on second joystick.
		Joystick2Button17 = 387,
		//
		// ���:
		//     Button 18 on second joystick.
		Joystick2Button18 = 388,
		//
		// ���:
		//     Button 19 on second joystick.
		Joystick2Button19 = 389,
		//
		// ���:
		//     Button 0 on third joystick.
		Joystick3Button0 = 390,
		//
		// ���:
		//     Button 1 on third joystick.
		Joystick3Button1 = 391,
		//
		// ���:
		//     Button 2 on third joystick.
		Joystick3Button2 = 392,
		//
		// ���:
		//     Button 3 on third joystick.
		Joystick3Button3 = 393,
		//
		// ���:
		//     Button 4 on third joystick.
		Joystick3Button4 = 394,
		//
		// ���:
		//     Button 5 on third joystick.
		Joystick3Button5 = 395,
		//
		// ���:
		//     Button 6 on third joystick.
		Joystick3Button6 = 396,
		//
		// ���:
		//     Button 7 on third joystick.
		Joystick3Button7 = 397,
		//
		// ���:
		//     Button 8 on third joystick.
		Joystick3Button8 = 398,
		//
		// ���:
		//     Button 9 on third joystick.
		Joystick3Button9 = 399,
		//
		// ���:
		//     Button 10 on third joystick.
		Joystick3Button10 = 400,
		//
		// ���:
		//     Button 11 on third joystick.
		Joystick3Button11 = 401,
		//
		// ���:
		//     Button 12 on third joystick.
		Joystick3Button12 = 402,
		//
		// ���:
		//     Button 13 on third joystick.
		Joystick3Button13 = 403,
		//
		// ���:
		//     Button 14 on third joystick.
		Joystick3Button14 = 404,
		//
		// ���:
		//     Button 15 on third joystick.
		Joystick3Button15 = 405,
		//
		// ���:
		//     Button 16 on third joystick.
		Joystick3Button16 = 406,
		//
		// ���:
		//     Button 17 on third joystick.
		Joystick3Button17 = 407,
		//
		// ���:
		//     Button 18 on third joystick.
		Joystick3Button18 = 408,
		//
		// ���:
		//     Button 19 on third joystick.
		Joystick3Button19 = 409,
		//
		// ���:
		//     Button 0 on forth joystick.
		Joystick4Button0 = 410,
		//
		// ���:
		//     Button 1 on forth joystick.
		Joystick4Button1 = 411,
		//
		// ���:
		//     Button 2 on forth joystick.
		Joystick4Button2 = 412,
		//
		// ���:
		//     Button 3 on forth joystick.
		Joystick4Button3 = 413,
		//
		// ���:
		//     Button 4 on forth joystick.
		Joystick4Button4 = 414,
		//
		// ���:
		//     Button 5 on forth joystick.
		Joystick4Button5 = 415,
		//
		// ���:
		//     Button 6 on forth joystick.
		Joystick4Button6 = 416,
		//
		// ���:
		//     Button 7 on forth joystick.
		Joystick4Button7 = 417,
		//
		// ���:
		//     Button 8 on forth joystick.
		Joystick4Button8 = 418,
		//
		// ���:
		//     Button 9 on forth joystick.
		Joystick4Button9 = 419,
		//
		// ���:
		//     Button 10 on forth joystick.
		Joystick4Button10 = 420,
		//
		// ���:
		//     Button 11 on forth joystick.
		Joystick4Button11 = 421,
		//
		// ���:
		//     Button 12 on forth joystick.
		Joystick4Button12 = 422,
		//
		// ���:
		//     Button 13 on forth joystick.
		Joystick4Button13 = 423,
		//
		// ���:
		//     Button 14 on forth joystick.
		Joystick4Button14 = 424,
		//
		// ���:
		//     Button 15 on forth joystick.
		Joystick4Button15 = 425,
		//
		// ���:
		//     Button 16 on forth joystick.
		Joystick4Button16 = 426,
		//
		// ���:
		//     Button 17 on forth joystick.
		Joystick4Button17 = 427,
		//
		// ���:
		//     Button 18 on forth joystick.
		Joystick4Button18 = 428,
		//
		// ���:
		//     Button 19 on forth joystick.
		Joystick4Button19 = 429,
		//
		// ���:
		//     Button 0 on fifth joystick.
		Joystick5Button0 = 430,
		//
		// ���:
		//     Button 1 on fifth joystick.
		Joystick5Button1 = 431,
		//
		// ���:
		//     Button 2 on fifth joystick.
		Joystick5Button2 = 432,
		//
		// ���:
		//     Button 3 on fifth joystick.
		Joystick5Button3 = 433,
		//
		// ���:
		//     Button 4 on fifth joystick.
		Joystick5Button4 = 434,
		//
		// ���:
		//     Button 5 on fifth joystick.
		Joystick5Button5 = 435,
		//
		// ���:
		//     Button 6 on fifth joystick.
		Joystick5Button6 = 436,
		//
		// ���:
		//     Button 7 on fifth joystick.
		Joystick5Button7 = 437,
		//
		// ���:
		//     Button 8 on fifth joystick.
		Joystick5Button8 = 438,
		//
		// ���:
		//     Button 9 on fifth joystick.
		Joystick5Button9 = 439,
		//
		// ���:
		//     Button 10 on fifth joystick.
		Joystick5Button10 = 440,
		//
		// ���:
		//     Button 11 on fifth joystick.
		Joystick5Button11 = 441,
		//
		// ���:
		//     Button 12 on fifth joystick.
		Joystick5Button12 = 442,
		//
		// ���:
		//     Button 13 on fifth joystick.
		Joystick5Button13 = 443,
		//
		// ���:
		//     Button 14 on fifth joystick.
		Joystick5Button14 = 444,
		//
		// ���:
		//     Button 15 on fifth joystick.
		Joystick5Button15 = 445,
		//
		// ���:
		//     Button 16 on fifth joystick.
		Joystick5Button16 = 446,
		//
		// ���:
		//     Button 17 on fifth joystick.
		Joystick5Button17 = 447,
		//
		// ���:
		//     Button 18 on fifth joystick.
		Joystick5Button18 = 448,
		//
		// ���:
		//     Button 19 on fifth joystick.
		Joystick5Button19 = 449,
		//
		// ���:
		//     Button 0 on sixth joystick.
		Joystick6Button0 = 450,
		//
		// ���:
		//     Button 1 on sixth joystick.
		Joystick6Button1 = 451,
		//
		// ���:
		//     Button 2 on sixth joystick.
		Joystick6Button2 = 452,
		//
		// ���:
		//     Button 3 on sixth joystick.
		Joystick6Button3 = 453,
		//
		// ���:
		//     Button 4 on sixth joystick.
		Joystick6Button4 = 454,
		//
		// ���:
		//     Button 5 on sixth joystick.
		Joystick6Button5 = 455,
		//
		// ���:
		//     Button 6 on sixth joystick.
		Joystick6Button6 = 456,
		//
		// ���:
		//     Button 7 on sixth joystick.
		Joystick6Button7 = 457,
		//
		// ���:
		//     Button 8 on sixth joystick.
		Joystick6Button8 = 458,
		//
		// ���:
		//     Button 9 on sixth joystick.
		Joystick6Button9 = 459,
		//
		// ���:
		//     Button 10 on sixth joystick.
		Joystick6Button10 = 460,
		//
		// ���:
		//     Button 11 on sixth joystick.
		Joystick6Button11 = 461,
		//
		// ���:
		//     Button 12 on sixth joystick.
		Joystick6Button12 = 462,
		//
		// ���:
		//     Button 13 on sixth joystick.
		Joystick6Button13 = 463,
		//
		// ���:
		//     Button 14 on sixth joystick.
		Joystick6Button14 = 464,
		//
		// ���:
		//     Button 15 on sixth joystick.
		Joystick6Button15 = 465,
		//
		// ���:
		//     Button 16 on sixth joystick.
		Joystick6Button16 = 466,
		//
		// ���:
		//     Button 17 on sixth joystick.
		Joystick6Button17 = 467,
		//
		// ���:
		//     Button 18 on sixth joystick.
		Joystick6Button18 = 468,
		//
		// ���:
		//     Button 19 on sixth joystick.
		Joystick6Button19 = 469,
		//
		// ���:
		//     Button 0 on seventh joystick.
		Joystick7Button0 = 470,
		//
		// ���:
		//     Button 1 on seventh joystick.
		Joystick7Button1 = 471,
		//
		// ���:
		//     Button 2 on seventh joystick.
		Joystick7Button2 = 472,
		//
		// ���:
		//     Button 3 on seventh joystick.
		Joystick7Button3 = 473,
		//
		// ���:
		//     Button 4 on seventh joystick.
		Joystick7Button4 = 474,
		//
		// ���:
		//     Button 5 on seventh joystick.
		Joystick7Button5 = 475,
		//
		// ���:
		//     Button 6 on seventh joystick.
		Joystick7Button6 = 476,
		//
		// ���:
		//     Button 7 on seventh joystick.
		Joystick7Button7 = 477,
		//
		// ���:
		//     Button 8 on seventh joystick.
		Joystick7Button8 = 478,
		//
		// ���:
		//     Button 9 on seventh joystick.
		Joystick7Button9 = 479,
		//
		// ���:
		//     Button 10 on seventh joystick.
		Joystick7Button10 = 480,
		//
		// ���:
		//     Button 11 on seventh joystick.
		Joystick7Button11 = 481,
		//
		// ���:
		//     Button 12 on seventh joystick.
		Joystick7Button12 = 482,
		//
		// ���:
		//     Button 13 on seventh joystick.
		Joystick7Button13 = 483,
		//
		// ���:
		//     Button 14 on seventh joystick.
		Joystick7Button14 = 484,
		//
		// ���:
		//     Button 15 on seventh joystick.
		Joystick7Button15 = 485,
		//
		// ���:
		//     Button 16 on seventh joystick.
		Joystick7Button16 = 486,
		//
		// ���:
		//     Button 17 on seventh joystick.
		Joystick7Button17 = 487,
		//
		// ���:
		//     Button 18 on seventh joystick.
		Joystick7Button18 = 488,
		//
		// ���:
		//     Button 19 on seventh joystick.
		Joystick7Button19 = 489,
		//
		// ���:
		//     Button 0 on eighth joystick.
		Joystick8Button0 = 490,
		//
		// ���:
		//     Button 1 on eighth joystick.
		Joystick8Button1 = 491,
		//
		// ���:
		//     Button 2 on eighth joystick.
		Joystick8Button2 = 492,
		//
		// ���:
		//     Button 3 on eighth joystick.
		Joystick8Button3 = 493,
		//
		// ���:
		//     Button 4 on eighth joystick.
		Joystick8Button4 = 494,
		//
		// ���:
		//     Button 5 on eighth joystick.
		Joystick8Button5 = 495,
		//
		// ���:
		//     Button 6 on eighth joystick.
		Joystick8Button6 = 496,
		//
		// ���:
		//     Button 7 on eighth joystick.
		Joystick8Button7 = 497,
		//
		// ���:
		//     Button 8 on eighth joystick.
		Joystick8Button8 = 498,
		//
		// ���:
		//     Button 9 on eighth joystick.
		Joystick8Button9 = 499,
		//
		// ���:
		//     Button 10 on eighth joystick.
		Joystick8Button10 = 500,
		//
		// ���:
		//     Button 11 on eighth joystick.
		Joystick8Button11 = 501,
		//
		// ���:
		//     Button 12 on eighth joystick.
		Joystick8Button12 = 502,
		//
		// ���:
		//     Button 13 on eighth joystick.
		Joystick8Button13 = 503,
		//
		// ���:
		//     Button 14 on eighth joystick.
		Joystick8Button14 = 504,
		//
		// ���:
		//     Button 15 on eighth joystick.
		Joystick8Button15 = 505,
		//
		// ���:
		//     Button 16 on eighth joystick.
		Joystick8Button16 = 506,
		//
		// ���:
		//     Button 17 on eighth joystick.
		Joystick8Button17 = 507,
		//
		// ���:
		//     Button 18 on eighth joystick.
		Joystick8Button18 = 508,
		//
		// ���:
		//     Button 19 on eighth joystick.
		Joystick8Button19 = 509
	};
}

struct st_Vector2
{
	static constexpr float PI = { 3.14159265358979323846f };

	float _X;
	float _Y;

	st_Vector2()
	{
		_X = 0;
		_Y = 0;
	}

	st_Vector2(float X, float Y)
	{
		_X = X;
		_Y = Y;
	}

	static st_Vector2 Up() { return st_Vector2(0, 1.0f); }
	static st_Vector2 Down() { return st_Vector2(0, -1.0f); }
	static st_Vector2 Left() { return st_Vector2(-1.0f, 0); }
	static st_Vector2 Right() { return st_Vector2(1.0f, 0); }
	static st_Vector2 Zero() { return st_Vector2(0, 0); }

	st_Vector2 operator + (st_Vector2& Vector)
	{
		return st_Vector2(_X + Vector._X, _Y + Vector._Y);
	}

	st_Vector2 operator - (st_Vector2& Vector)
	{
		return st_Vector2(_X - Vector._X, _Y - Vector._Y);
	}

	st_Vector2 operator - (st_Vector2&& Vector)
	{
		return st_Vector2(_X - Vector._X, _Y - Vector._Y);
	}

	st_Vector2 operator * (int8& Sclar)
	{
		return st_Vector2(_X * Sclar, _Y * Sclar);
	}

	st_Vector2 operator * (float& Sclar)
	{
		return st_Vector2(_X * Sclar, _Y * Sclar);
	}

	// �Ÿ� ���ϱ�
	static float Distance(st_Vector2 TargetCellPosition, st_Vector2 MyCellPosition)
	{
		return sqrt(((TargetCellPosition._X - MyCellPosition._X) * (TargetCellPosition._X - MyCellPosition._X))
			+ ((TargetCellPosition._Y - MyCellPosition._Y) * (TargetCellPosition._Y - MyCellPosition._Y)));
	}

	// ���� ũ�� ���ϱ�
	float Size(st_Vector2 Vector)
	{
		return sqrt(SizeSquared());
	}

	float SizeSquared()
	{
		return _X * _X + _Y * _Y;
	}

	// ���� ����ȭ
	st_Vector2 Normalize()
	{
		/*float VectorSize = Size(*this);

		return st_Vector2(_X / VectorSize, _Y / VectorSize);*/
		float SquaredSum = SizeSquared();

		if (SquaredSum != 0)
		{
			const __m128 fOneHalf = _mm_set_ss(0.5f);
			__m128 Y0, X0, X1, X2, FOver2;
			float temp;

			Y0 = _mm_set_ss(SquaredSum);
			X0 = _mm_rsqrt_ss(Y0);	// 1/sqrt estimate (12 bits)
			FOver2 = _mm_mul_ss(Y0, fOneHalf);

			// 1st Newton-Raphson iteration
			X1 = _mm_mul_ss(X0, X0);
			X1 = _mm_sub_ss(fOneHalf, _mm_mul_ss(FOver2, X1));
			X1 = _mm_add_ss(X0, _mm_mul_ss(X0, X1));

			// 2nd Newton-Raphson iteration
			X2 = _mm_mul_ss(X1, X1);
			X2 = _mm_sub_ss(fOneHalf, _mm_mul_ss(FOver2, X2));
			X2 = _mm_add_ss(X1, _mm_mul_ss(X1, X2));

			_mm_store_ss(&temp, X2);

			return st_Vector2(_X, _Y) * temp;
		}
		else
		{
			return st_Vector2::Zero();
		}
	}

	static en_MoveDir GetMoveDir(st_Vector2 NormalVector)
	{
		NormalVector._X = round(NormalVector._X);
		NormalVector._Y = round(NormalVector._Y);

		if (NormalVector._X < 0)
		{
			return en_MoveDir::LEFT;
		}

		if (NormalVector._X > 0)
		{
			return en_MoveDir::RIGHT;
		}

		if (NormalVector._Y > 0)
		{
			return en_MoveDir::UP;
		}

		if (NormalVector._Y < 0)
		{
			return en_MoveDir::DOWN;
		}
	}

	float Dot(st_Vector2 InVector)
	{		
		return _X * InVector._X + _Y * InVector._Y;
	}

	static st_Vector2 GetForwardVector(en_MoveDir MoveDir)
	{
		switch (MoveDir)
		{
		case en_MoveDir::UP:
			return st_Vector2(0, 1.0f);
		case en_MoveDir::DOWN:
			return st_Vector2(0, -1.0f);
		case en_MoveDir::LEFT:
			return st_Vector2(-1.0f, 0);
		case en_MoveDir::RIGHT:
			return st_Vector2(1.0f, 0);
		}
	}

	static float GetAngle(st_Vector2 TargetVector, st_Vector2 MyVector)
	{
		st_Vector2 TargetVectorNormal = TargetVector.Normalize();
		st_Vector2 MyVectorNormal = MyVector.Normalize();

		float Dot = TargetVectorNormal.Dot(MyVectorNormal);
		float ACosf = acosf(Dot);
		float Angle = ACosf * 180.0f / st_Vector2::PI;

		return Angle;
	}

	static bool AngleCheck(st_Vector2 CheckPosition, st_Vector2 APosition, st_Vector2 BPosition, float Angle)
	{
		st_Vector2 APositionNormal = (APosition - CheckPosition).Normalize();
		st_Vector2 BPositionNormal = (BPosition - CheckPosition).Normalize();

		float APositionBPositionDot = APositionNormal.Dot(BPositionNormal);
		float APositionBPositionACosf = acosf(APositionBPositionDot);
		float APositionBPositionAngle = APositionBPositionACosf * 180.0f / st_Vector2::PI;

		if (APositionBPositionAngle >= 0 && APositionBPositionAngle <= Angle)
		{
			return true;
		}
	}

	static bool CheckFieldOfView(st_Vector2 TargetPosition, st_Vector2 CheckPosition, en_MoveDir Dir, int16 Angle)
	{
		// ��� ���� ���� ���ϱ�
		st_Vector2 TargetDirVector = TargetPosition - CheckPosition;
		// ���� ���� ����ȭ
		st_Vector2 NormalizeDirVector = TargetDirVector.Normalize();
				
		float FovCos = cosf((Angle * 0.5f) * PI / 180.0f);

		st_Vector2 ForwardVector;

		switch (Dir)
		{
		case en_MoveDir::UP:
			ForwardVector._X = 0;
			ForwardVector._Y = 1.0f;
			break;
		case en_MoveDir::DOWN:
			ForwardVector._X = 0;
			ForwardVector._Y = -1.0f;
			break;
		case en_MoveDir::LEFT:
			ForwardVector._X = -1.0f;
			ForwardVector._Y = 0;
			break;
		case en_MoveDir::RIGHT:
			ForwardVector._X = 1.0f;
			ForwardVector._Y = 0;
			break;
		}

		if (NormalizeDirVector.Dot(ForwardVector) >= FovCos)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
};

struct st_Vector2Int
{
	int32 _X;
	int32 _Y;

	st_Vector2Int()
	{
		_X = 0;
		_Y = 0;
	}

	st_Vector2Int(int32 X, int32 Y)
	{
		_X = X;
		_Y = Y;
	}

	// ���� ����
	static st_Vector2Int Up() { return st_Vector2Int(0, 1); }
	static st_Vector2Int Down() { return st_Vector2Int(0, -1); }
	static st_Vector2Int Left() { return st_Vector2Int(-1, 0); }
	static st_Vector2Int Right() { return st_Vector2Int(1, 0); }
	static st_Vector2Int Zero() { return st_Vector2Int(0, 0); }

	st_Vector2Int operator +(st_Vector2Int& Vector)
	{
		return st_Vector2Int(_X + Vector._X, _Y + Vector._Y);
	}

	st_Vector2Int operator -(st_Vector2Int& Vector)
	{
		return st_Vector2Int(_X - Vector._X, _Y - Vector._Y);
	}

	st_Vector2Int operator *(int8 Value)
	{
		return st_Vector2Int(_X * Value, _Y * Value);
	}

	bool operator == (st_Vector2Int CellPosition)
	{
		if (_X == CellPosition._X && _Y == CellPosition._Y)
		{
			return true;
		}

		return false;
	}

	int CellDistanceFromZero()
	{
		return abs(_X) + abs(_Y);
	}

	// �Ÿ� ���ϱ�
	static int16 Distance(st_Vector2Int TargetCellPosition, st_Vector2Int MyCellPosition)
	{
		return (int16)sqrt(pow(TargetCellPosition._X - MyCellPosition._X, 2) + pow(TargetCellPosition._Y - MyCellPosition._Y, 2));
	}

	static en_MoveDir GetMoveDir(st_Vector2Int NormalVector)
	{
		if (NormalVector._X > 0)
		{
			return en_MoveDir::RIGHT;
		}

		if (NormalVector._X < 0)
		{
			return en_MoveDir::LEFT;
		}

		if (NormalVector._Y > 0)
		{
			return en_MoveDir::UP;
		}

		if (NormalVector._Y < 0)
		{
			return en_MoveDir::DOWN;
		}
	}
};


struct st_PositionInfo
{
	en_CreatureState State;
	st_Vector2Int CollisionPosition;
	st_Vector2 Position;
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
	int8 ObjectStep;
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
	static st_Color Yellow() { return st_Color(255, 212, 255); }
	static st_Color White() { return st_Color(255, 255, 255); }
};

struct st_CraftingMaterialItemInfo
{
	en_SmallItemCategory MaterialItemType; // ����� ����
	wstring MaterialItemName; // ����� �̸�
	int16 ItemCount; // ����� �ʿ� ����
};

struct st_CraftingCompleteItem
{
	en_UIObjectInfo OwnerCraftingTable;  // �ϼ� �������� ������ ���۴�
	en_SmallItemCategory CompleteItemType; // �ϼ� ������ ����
	wstring CompleteItemName; // �ϼ� ������ �̸�
	vector<st_CraftingMaterialItemInfo> Materials; // ������ ���鶧 �ʿ��� ����
};

struct st_CraftingItemCategory
{
	en_LargeItemCategory CategoryType; // ������ ����
	wstring CategoryName; // ������ ���� �̸�
	vector<CItem*> CommonCraftingCompleteItems; // ���ֿ� ���� �ϼ� �����۵�
};

struct st_ItemInfo
{
	int64 ItemDBId;							  // ������ DB�� ����Ǿ� �ִ� ID		
	int64 InventoryItemNumber;				  // �������� �κ��丮�� ���Ҷ� ������ ����
	bool ItemIsQuickSlotUse;				  // �����Կ� ��ϵǾ� �ִ��� ���� 
	bool Rotated;							  // �������� ȸ�� �Ǿ� �ִ��� �ƴ��� ����
	int16 Width;			     			  // ������ �ʺ�
	int16 Height;							  // ������ ����	
	int16 TileGridPositionX;				  // �κ��丮 ��ġ X
	int16 TileGridPositionY;				  // �κ��丮 ��ġ Y
	en_UIObjectInfo OwnerCraftingTable;		  // �������� ���� ������ �������̶�� �������� ���� ���۴�
	en_LargeItemCategory ItemLargeCategory;   // ������ ��з�
	en_MediumItemCategory ItemMediumCategory; // ������ �ߺз�
	en_SmallItemCategory ItemSmallCategory;	  // ������ �Һз�
	int32 MaxHP;							  // ������ �ִ� ������
	int32 CurrentHP;						  // ������ ���� ������
	int32 CraftingMaxHP;					  // ������ �ִ� ä�� HP
	int32 CraftingCurrentHP;				  // ������ ���� ä�� HP
	wstring ItemName;			              // ������ �̸�
	wstring ItemExplain;		              // ������ ����
	int64 ItemCraftingTime;					  // ������ ���� �ð�
	int64 ItemCraftingRemainTime;			  // ������ ���� ���� �ð�
	int32 ItemMinDamage;			          // ������ �ּ� ���ݷ�
	int32 ItemMaxDamage;			          // ������ �ִ� ���ݷ�
	int32 ItemDefence;				          // ������ ����
	int32 ItemMaxCount;				          // �������� ���� �� �� �ִ� �ִ� ����
	int16 ItemCount;			              // ����		
	bool ItemIsEquipped;			          // �������� ������ �� �ִ���		
	vector<st_CraftingMaterialItemInfo> Materials; // ���� �������� ��� ���տ� �ʿ��� ��� ������ ���

	st_ItemInfo()
	{
		ItemDBId = 0;
		ItemIsQuickSlotUse = false;
		Width = 0;
		Height = 0;
		Rotated = false;
		TileGridPositionX = 0;
		TileGridPositionY = 0;
		OwnerCraftingTable = en_UIObjectInfo::UI_OBJECT_INFO_NONE;
		ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_NONE;
		ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_NONE;
		ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_NONE;
		MaxHP = 0;
		CurrentHP = 0;
		CraftingMaxHP = 0;
		CraftingCurrentHP = 0;
		ItemName = L"";
		ItemExplain = L"";
		ItemCraftingTime = 0;
		ItemCraftingRemainTime = 0;
		ItemMinDamage = 0;
		ItemMaxDamage = 0;
		ItemDefence = 0;
		ItemMaxCount = 0;
		ItemCount = 0;				
		ItemIsEquipped = false;
	}

	bool operator == (st_ItemInfo OtherItemInfo)
	{
		if (TileGridPositionX == OtherItemInfo.TileGridPositionX
			&& TileGridPositionY == OtherItemInfo.TileGridPositionY
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
	bool IsSkillLearn;       // ��ų�� ��������� ���� ����
	bool CanSkillUse;		 // ��ų�� ��� �� �� �ִ��� ����	
	bool IsQuickSlotUse;	 // �����Կ� ��ϵǾ� �ִ��� ����	
	en_SkillLargeCategory SkillLargeCategory; // ��ų ��з�
	en_SkillMediumCategory SkillMediumCategory; // ��ų �ߺз�
	en_SkillType SkillType;	 // ��ų ����
	int8 SkillLevel;		 // ��ų ����
	wstring SkillName;		 // ��ų �̸�
	int32 SkillCoolTime;	 // ��ų ��Ÿ��		
	int32 SkillCastingTime;  // ��ų ĳ���� Ÿ��
	int64 SkillDurationTime; // ��ų ���� �ð�
	int64 SkillDotTime;      // ��ų ��Ʈ �ð� 
	int64 SkillRemainTime;   // ��ų ���� �ð�
	float SkillTargetEffectTime;
	en_SkillType NextComboSkill;
	map<en_MoveDir, wstring> SkillAnimations; // ��ų �ִϸ��̼�	
	wstring SkillExplanation; // ��ų ���� 

	st_SkillInfo()
	{
		IsSkillLearn = false;
		CanSkillUse = true;
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
		NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
		SkillTargetEffectTime = 0;
		SkillExplanation = L"";
	}
};

struct st_AttackSkillInfo : public st_SkillInfo
{
	int32 SkillMinDamage;		// �ּ� ���ݷ�
	int32 SkillMaxDamage;		// �ִ� ���ݷ�
	bool SkillDebuf;			// ��ų ����� ����	
	int8 SkillDebufAttackSpeed; // ��ų ���ݼӵ� ���� ��ġ
	int8 SkillDebufMovingSpeed; // ��ų �̵��ӵ� ���� ��ġ
	bool SkillDebufStun;		// ��ų ���� ����
	bool SkillDebufPushAway;	// ��ų �з��� ����
	bool SkillDebufRoot;		// ��ų �̵��Ұ� ����	
	int64 SkillDamageOverTime;	// ��ų ��Ʈ ������ �ð� ����
	int8 StatusAbnormalityProbability; // ���� �̻� ���� Ȯ��

	st_AttackSkillInfo()
	{
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
	}
};

struct st_TacTicSkillInfo : public st_SkillInfo
{

};

struct st_HealSkillInfo : public st_TacTicSkillInfo
{
	int32 SkillMinHealPoint; // �ּ� ġ����
	int32 SkillMaxHealPoint; // �ִ� ġ����

	st_HealSkillInfo()
	{
		SkillMinHealPoint = 0;
		SkillMaxHealPoint = 0;
	}
};

struct st_BufSkillInfo : public st_SkillInfo
{
	int32 IncreaseMinAttackPoint; // �����ϴ� �ּ� ���� ���ݷ�
	int32 IncreaseMaxAttackPoint; // �����ϴ� �ִ� ���� ���ݷ�
	int32 IncreaseMeleeAttackSpeedPoint; // �����ϴ� ���� ���� �ӵ�
	int16 IncreaseMeleeAttackHitRate; // �����ϴ� ���� ���߷�	
	int16 IncreaseMagicAttackPoint; // �����ϴ� ���� ���ݷ�
	int16 IncreaseMagicCastingPoint; // �����ϴ� ���� ĳ���� �ӵ�
	int16 IncreaseMagicAttackHitRate; // �����ϴ� ���� ���߷�		
	int32 IncreaseDefencePoint; // �����ϴ� ���� 
	int16 IncreaseEvasionRate; // �����ϴ� ȸ����
	int16 IncreaseMeleeCriticalPoint; // �����ϴ� ���� ġ��Ÿ��
	int16 IncreaseMagicCriticalPoint; // �����ϴ� ���� ġ��Ÿ��
	float IncreaseSpeedPoint; // �����ϴ� �̵� �ӵ�	
	int16 IncreaseStatusAbnormalityResistance; // �����ϴ� �����̻����װ�

	st_BufSkillInfo()
	{
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
	}
};

struct st_QuickSlotBarSlotInfo
{
	int64 AccountDBId;               // ������ ���� ������ Account
	int64 PlayerDBId;                // ������ ���� ������ Player	
	int8 QuickSlotBarIndex;          // ������ Index
	int8 QuickSlotBarSlotIndex;      // ������ ���� Index
	int16 QuickSlotKey;              // �����Կ� ������ Ű��
	CSkill* QuickBarSkill = nullptr; // �����Կ� ����� ��ų ����	
	bool CanQuickSlotUse = true;     // �������� ����� �� �ִ��� ������
};

struct st_QuickSlotBarPosition
{
	int8 QuickSlotBarIndex;
	int8 QuickSlotBarSlotIndex;
};


// ���۴� ����ǰ ����
struct st_CraftingTableRecipe
{
	en_GameObjectType CraftingTableType; // ���۴� ����
	wstring CraftingTableName;			 // ���۴� �̸�	
	vector<CItem*> CraftingTableCompleteItems;	// ���۴� ����ǰ ���
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
	int64 MapID;
	wstring MapName;
	int32 MapSectorSize;
	int8 ChannelCount;
};

struct st_TileMapInfo
{	
	en_MapTileInfo MapTileType;
	int64 AccountID;
	int64 PlayerID;
	st_Vector2Int TilePosition;
};

struct st_OptionItemInfo
{
	en_OptionType OptionType;
	wstring OptionName;
};