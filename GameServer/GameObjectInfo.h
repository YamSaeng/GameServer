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

	// 거리 구하기
	static float Distance(st_Vector2 TargetCellPosition, st_Vector2 MyCellPosition)
	{
		return sqrt(((TargetCellPosition._X - MyCellPosition._X) * (TargetCellPosition._X - MyCellPosition._X))
			+ ((TargetCellPosition._Y - MyCellPosition._Y) * (TargetCellPosition._Y - MyCellPosition._Y)));
	}

	// 벡터 크기 구하기
	float Size(st_Vector2 Vector)
	{
		return sqrt(SizeSquared());
	}

	float SizeSquared()
	{
		return _X * _X + _Y * _Y;
	}

	// 벡터 정규화
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
		// 대상 방향 벡터 구하기
		st_Vector2 TargetDirVector = TargetPosition - CheckPosition;
		// 방향 벡터 정규화
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

	// 방향 벡터
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

	// 거리 구하기
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
	bool ItemIsQuickSlotUse;				  // 퀵슬롯에 등록되어 있는지 여부 
	bool Rotated;							  // 아이템이 회전 되어 있는지 아닌지 여부
	int16 Width;			     			  // 아이템 너비
	int16 Height;							  // 아이템 높이	
	int16 TileGridPositionX;				  // 인벤토리 위치 X
	int16 TileGridPositionY;				  // 인벤토리 위치 Y
	en_UIObjectInfo OwnerCraftingTable;		  // 아이템이 제작 가능한 아이템이라면 아이템이 속한 제작대
	en_LargeItemCategory ItemLargeCategory;   // 아이템 대분류
	en_MediumItemCategory ItemMediumCategory; // 아이템 중분류
	en_SmallItemCategory ItemSmallCategory;	  // 아이템 소분류
	int32 MaxHP;							  // 아이템 최대 내구도
	int32 CurrentHP;						  // 아이템 현재 내구도
	int32 CraftingMaxHP;					  // 아이템 최대 채집 HP
	int32 CraftingCurrentHP;				  // 아이템 현재 채집 HP
	wstring ItemName;			              // 아이템 이름
	wstring ItemExplain;		              // 아이템 설명문
	int64 ItemCraftingTime;					  // 아이템 제작 시간
	int64 ItemCraftingRemainTime;			  // 아이템 제작 남은 시간
	int32 ItemMinDamage;			          // 아이템 최소 공격력
	int32 ItemMaxDamage;			          // 아이템 최대 공격력
	int32 ItemDefence;				          // 아이템 방어력
	int32 ItemMaxCount;				          // 아이템을 소유 할 수 있는 최대 개수
	int16 ItemCount;			              // 개수		
	bool ItemIsEquipped;			          // 아이템을 착용할 수 있는지		
	vector<st_CraftingMaterialItemInfo> Materials; // 제작 아이템일 경우 조합에 필요한 재료 아이템 목록

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
	bool IsSkillLearn;       // 스킬을 배웠는지에 대한 여부
	bool CanSkillUse;		 // 스킬을 사용 할 수 있는지 여부	
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
	en_SkillType NextComboSkill;
	map<en_MoveDir, wstring> SkillAnimations; // 스킬 애니메이션	
	wstring SkillExplanation; // 스킬 설명 

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
	int32 SkillMinDamage;		// 최소 공격력
	int32 SkillMaxDamage;		// 최대 공격력
	bool SkillDebuf;			// 스킬 디버프 여부	
	int8 SkillDebufAttackSpeed; // 스킬 공격속도 감소 수치
	int8 SkillDebufMovingSpeed; // 스킬 이동속도 감소 수치
	bool SkillDebufStun;		// 스킬 스턴 여부
	bool SkillDebufPushAway;	// 스킬 밀려남 여부
	bool SkillDebufRoot;		// 스킬 이동불가 여부	
	int64 SkillDamageOverTime;	// 스킬 도트 데미지 시간 간격
	int8 StatusAbnormalityProbability; // 상태 이상 적용 확률

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
	int32 SkillMinHealPoint; // 최소 치유량
	int32 SkillMaxHealPoint; // 최대 치유량

	st_HealSkillInfo()
	{
		SkillMinHealPoint = 0;
		SkillMaxHealPoint = 0;
	}
};

struct st_BufSkillInfo : public st_SkillInfo
{
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
	int64 AccountDBId;               // 퀵슬롯 슬롯 소유한 Account
	int64 PlayerDBId;                // 퀵슬롯 슬롯 소유한 Player	
	int8 QuickSlotBarIndex;          // 퀵슬롯 Index
	int8 QuickSlotBarSlotIndex;      // 퀵슬롯 슬롯 Index
	int16 QuickSlotKey;              // 퀵슬롯에 연동된 키값
	CSkill* QuickBarSkill = nullptr; // 퀵슬롯에 등록할 스킬 정보	
	bool CanQuickSlotUse = true;     // 퀵슬롯을 사용할 수 있는지 없는지
};

struct st_QuickSlotBarPosition
{
	int8 QuickSlotBarIndex;
	int8 QuickSlotBarSlotIndex;
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