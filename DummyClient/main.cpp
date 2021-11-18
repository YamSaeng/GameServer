#pragma comment(lib,"winmm")
#pragma comment(lib,"ws2_32")
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <xstring>
#include <vector>
#include "RingBuffer.h"
#include "Message.h"
#include "CommonProtocol.h"
#include "LockFreeQue.h"
#include <process.h>

using namespace std;

#pragma region 구조체, 열거형
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
	STUN,
	PUSH_AWAY,
	ROOT
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
	ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_FIERCE_ATTACK = 300,
	ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CONVERSION_ATTACK = 301,
	ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_SHAEHONE_ATTACK = 302,
	ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHOHONE_ATTACK = 303,
	ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_SMASH_WAVE_ATTACK = 304,
	ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHARGE_POSE = 305,
	ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_FLAME_HARPOON = 306,
	ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_HELL_FIRE = 307,
	ITEM_SMALL_CATEOGRY_SKILLBOOK_SHAMAN_HEALING_LIGHT = 308,
	ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_HEALING_WIND = 309,
	ITEM_SMALL_CATEGORY_SKILLBOOK_SHOCK_RELEASE = 310,

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
	SKILL_NORMAL = 1,

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
	EFFECT_OBJECT_SPAWN,
	EFFECT_DEBUF_ROOT,
	EFFECT_DEBUF_STUN
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

enum class en_DotType : int8
{
	DOT_TYPE_NONE = 0,
	DOT_TYPE_AUTO_RECOVERY,
	DOT_TYPE_HEAL,
	DOT_TYPE_POISON,
	DOT_TYPE_BLEEDING,
	DOT_TYPE_BURN
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
	bool ItemIsQuickSlotUse;        // 퀵슬롯에 등록되어 있는지 여부 
	en_LargeItemCategory ItemLargeCategory; // 아이템 대분류
	en_MediumItemCategory ItemMediumCategory; // 아이템 중분류
	en_SmallItemCategory ItemSmallCategory;		// 아이템 소분류
	wstring ItemName;			// 아이템 이름
	wstring ItemExplain;			// 아이템 설명문
	int32 ItemMinDamage;			// 아이템 최소 공격력
	int32 ItemMaxDamage;			// 아이템 최대 공격력
	int32 ItemDefence;				// 아이템 방어력
	int32 ItemMaxCount;				// 아이템을 소유 할 수 있는 최대 개수
	int16 ItemCount;			// 개수
	wstring ItemThumbnailImagePath; // 이미지 경로
	bool ItemIsEquipped;			// 아이템을 착용할 수 있는지	
	int8 ItemSlotIndex;				// 슬롯 번호

	st_ItemInfo()
	{
		ItemDBId = 0;
		ItemIsQuickSlotUse = false;
		ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_NONE;
		ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_NONE;
		ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_NONE;
		ItemName = L"";
		ItemExplain = L"";
		ItemMinDamage = 0;
		ItemMaxDamage = 0;
		ItemDefence = 0;
		ItemMaxCount = 0;
		ItemCount = 0;
		ItemThumbnailImagePath = L"";
		ItemIsEquipped = false;
		ItemSlotIndex = -1;
	}

	bool operator == (st_ItemInfo OtherItemInfo)
	{
		if (ItemSlotIndex == OtherItemInfo.ItemSlotIndex
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
	bool IsQuickSlotUse;	 // 퀵슬롯에 등록되어 있는지 여부
	en_SkillLargeCategory SkillLargeCategory; // 스킬 대분류
	en_SkillMediumCategory SkillMediumCategory; // 스킬 중분류
	en_SkillType SkillType;	 // 스킬 종류
	int8 SkillLevel;		 // 스킬 레벨
	wstring SkillName;		 // 스킬 이름
	int32 SkillCoolTime;	 // 스킬 쿨타임
	int32 SkillCastingTime;  // 스킬 캐스팅 타임
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
		SkillTargetEffectTime = 0;
		SkillImagePath = L"";
		CanSkillUse = true;
	}
};

struct st_AttackSkillInfo : public st_SkillInfo
{
	int32 SkillMinDamage;		// 최소 공격력
	int32 SkillMaxDamage;		// 최대 공격력
	bool SkillDebuf;			// 스킬 디버프 여부
	int64 SkillDebufTime;		// 스킬 디버프 시간
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
		SkillDebufTime = 0;
		SkillDebufAttackSpeed = 0;
		SkillDebufMovingSpeed = 0;
		SkillDebufStun = false;
		SkillDebufPushAway = false;
		SkillDebufRoot = false;
		SkillDamageOverTime = 0;
		StatusAbnormalityProbability = 0;
	}
};

struct st_HealSkillInfo : public st_SkillInfo
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
	int64 AccountDBId; // 퀵슬롯 슬롯 소유한 Account
	int64 PlayerDBId;  // 퀵슬롯 슬롯 소유한 Player	
	int8 QuickSlotBarIndex; // 퀵슬롯 Index
	int8 QuickSlotBarSlotIndex; // 퀵슬롯 슬롯 Index
	int16 QuickSlotKey;   // 퀵슬롯에 연동된 키값
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
#pragma endregion

enum en_ClientMessage
{
	CHAT_MSG,
	MOVE,
	MELEE_ATTACK
};

int64 G_DummyClientAccountId = 1000000;
int64 G_DummyClientLoginId;

struct st_Client
{
	SOCKET ClientSocket;
	SOCKADDR_IN ClientAddr;

	CLockFreeQue<CMessage*> SendRingBuf;
	CRingBuffer RecvRingBuf;

	int64 AccountId;
	wstring LoginId;	

	int32 X;
	int32 Y;

	WCHAR ChatMsg[256];
		
	bool IsLogin;
	bool IsEnterGame;

	st_GameObjectInfo MyCharacterGameObjectInfo;

	st_Client()
	{
		ClientSocket = INVALID_SOCKET;

		AccountId = 0;		

		X = 0;
		Y = 0;

		memset(ChatMsg, 0, sizeof(ChatMsg));

		IsLogin = false;
		IsEnterGame = false;
	}
};

static unsigned __stdcall SendProc(void* Argument);
static unsigned __stdcall SelectProc(void* Argument);

st_Client* G_ClientArray;
st_Client** G_SelectCheckArray;

#define CLIENT_MAX 500

int main()
{
	setlocale(LC_ALL, "Korean");

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		DWORD Error = WSAGetLastError();
		wprintf(L"WSAStartup Error %d\n", Error);
	}

	_beginthreadex(NULL, 0, SendProc, 0, 0, NULL);
	_beginthreadex(NULL, 0, SelectProc, 0, 0, NULL);
	
	while (1)
	{
		Sleep(1000);
	}
}

unsigned __stdcall SendProc(void* Argument)
{
	Sleep(10000);

	while (1)
	{
		for (int32 i = 0; i < CLIENT_MAX; i++)
		{
			// 로그인 및 게임에 입장하면 메세지 전송
			if (G_ClientArray[i].IsLogin == true && G_ClientArray[i].IsEnterGame == true)
			{
				int RandMessageType = rand() % 3;
				int RandDir;				
				CMessage* RandPacket = RandPacket = CMessage::Alloc();
				RandPacket->Clear();

				wstring SendChatMsg;
				int8 DummyChatLen;
				
				switch ((en_ClientMessage)RandMessageType)
				{
				case en_ClientMessage::CHAT_MSG:					
					*RandPacket << (int16)en_PACKET_C2S_MESSAGE;
					*RandPacket << G_ClientArray[i].AccountId;
					*RandPacket << G_ClientArray[i].MyCharacterGameObjectInfo.ObjectId;

					swprintf_s(G_ClientArray[i].ChatMsg, sizeof(G_ClientArray[i].ChatMsg), L"[%s]가 [%d]를 보냈습니다.", G_ClientArray[i].LoginId.c_str(), G_ClientArray[i].MyCharacterGameObjectInfo.ObjectId, RandMessageType * i);
					SendChatMsg = G_ClientArray[i].ChatMsg;

					DummyChatLen = (int8)(SendChatMsg.length() * 2);
					*RandPacket << DummyChatLen;
					RandPacket->InsertData(SendChatMsg.c_str(), DummyChatLen);

					G_ClientArray[i].SendRingBuf.Enqueue(RandPacket);
					break;
				case en_ClientMessage::MOVE:					
					*RandPacket << (int16)en_PACKET_C2S_MOVE;					
					*RandPacket << G_ClientArray[i].AccountId;
					*RandPacket << G_ClientArray[i].MyCharacterGameObjectInfo.ObjectId;

					RandDir = rand() % 4;
					*RandPacket << (int8)RandDir;

					G_ClientArray[i].SendRingBuf.Enqueue(RandPacket);
					break;	
				case en_ClientMessage::MELEE_ATTACK:
					break;
				}
			}
			else
			{
				
			}			
		}

		Sleep(1000);
	}

	return 0;
}

unsigned __stdcall SelectProc(void* Argument)
{
	int32 DummyId = 1;	

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(ServerAddr));
	ServerAddr.sin_family = AF_INET;
	InetPtonW(AF_INET, L"127.0.0.1", &ServerAddr.sin_addr);
	ServerAddr.sin_port = htons(7777);

	FD_SET ReadSet;
	FD_SET WriteSet;

	st_Client Client;
	G_ClientArray = new st_Client[CLIENT_MAX];
	G_SelectCheckArray = new st_Client * [FD_SETSIZE];
	memset(G_SelectCheckArray, 0, sizeof(st_Client*) * FD_SETSIZE);

	Sleep(5000);

	for (int i = 0; i < CLIENT_MAX; i++)
	{
		G_ClientArray[i].ClientSocket = socket(AF_INET, SOCK_STREAM, 0);
		G_ClientArray[i].AccountId = G_DummyClientAccountId + i;
		int ret = connect(G_ClientArray[i].ClientSocket, (SOCKADDR*)&ServerAddr, sizeof(G_ClientArray[i].ClientAddr));
		if (ret == SOCKET_ERROR)
		{
			DWORD Error = WSAGetLastError();
			wprintf(L"connect Error %d", Error);
		}
	}	

	while (1)
	{
		int SelectIndex = 0;

		for (int i = 0; i < CLIENT_MAX / FD_SETSIZE; i++)
		{
			FD_ZERO(&ReadSet);
			FD_ZERO(&WriteSet);

			// 검사할 소켓 셋팅
			for (int j = 0; j < FD_SETSIZE; j++)
			{
				FD_SET(G_ClientArray[SelectIndex].ClientSocket, &ReadSet);

				if (G_ClientArray[SelectIndex].SendRingBuf.GetUseSize() > 0)
				{
					FD_SET(G_ClientArray[SelectIndex].ClientSocket, &WriteSet);
				}

				G_SelectCheckArray[j] = &G_ClientArray[SelectIndex];

				SelectIndex++;
			}

			timeval Time;
			Time.tv_sec = 0;
			Time.tv_usec = 0;

			int SelectRetval = select(0, &ReadSet, &WriteSet, NULL, &Time);
			if (SelectRetval == SOCKET_ERROR)
			{
				DWORD Error = WSAGetLastError();
				wprintf(L"Select Error %d\n", Error);
			}

			if (SelectRetval > 0)
			{
				for (int k = 0; k < FD_SETSIZE; k++)
				{
					if (FD_ISSET(G_SelectCheckArray[k]->ClientSocket, &ReadSet))
					{
						int RecvDirectEnqueSize = G_SelectCheckArray[k]->RecvRingBuf.GetDirectEnqueueSize();
						int RecvRet = recv(G_SelectCheckArray[k]->ClientSocket, G_SelectCheckArray[k]->RecvRingBuf.GetRearBufferPtr(), RecvDirectEnqueSize, 0);
						if (RecvRet == SOCKET_ERROR)
						{
							DWORD Error = WSAGetLastError();
							if (Error != WSAEWOULDBLOCK)
							{
								break;
							}
						}
						else if (RecvRet == 0)
						{
							closesocket(G_SelectCheckArray[k]->ClientSocket);
						}

						G_SelectCheckArray[k]->RecvRingBuf.MoveRear(RecvRet);

						CMessage::st_ENCODE_HEADER Header;
						CMessage* RecvPacket = CMessage::Alloc();

						while (1)
						{
							RecvPacket->Clear();

							if (G_SelectCheckArray[k]->RecvRingBuf.GetUseSize() < sizeof(CMessage::st_ENCODE_HEADER))
							{
								break;
							}

							G_SelectCheckArray[k]->RecvRingBuf.Peek((char*)&Header, sizeof(CMessage::st_ENCODE_HEADER));
							if (Header.PacketLen + sizeof(CMessage::st_ENCODE_HEADER) <= G_SelectCheckArray[k]->RecvRingBuf.GetUseSize())
							{
								if (Header.PacketCode != 119)
								{
									break;
								}
							}
							else
							{
								break;
							}

							G_SelectCheckArray[k]->RecvRingBuf.MoveFront(sizeof(CMessage::st_ENCODE_HEADER));
							G_SelectCheckArray[k]->RecvRingBuf.Dequeue(RecvPacket->GetRearBufferPtr(), Header.PacketLen);
							RecvPacket->SetHeader((char*)&Header, sizeof(CMessage::st_ENCODE_HEADER));
							RecvPacket->MoveWritePosition(Header.PacketLen);

							if (!RecvPacket->Decode())
							{
								break;
							}

							WORD MessageType;
							*RecvPacket >> MessageType;

							switch (MessageType)
							{
							case en_PACKET_S2C_GAME_CLIENT_CONNECTED:
							{
								bool IsDummy = true;

								CMessage* ReqGameServerLoginPacket = CMessage::Alloc();

								ReqGameServerLoginPacket->Clear();
								WCHAR DummyCharacterName[256] = { 0 };
								swprintf_s(DummyCharacterName, sizeof(DummyCharacterName), L"Dummy_Player %d", G_DummyClientLoginId++);

								G_SelectCheckArray[k]->LoginId = DummyCharacterName;
								*ReqGameServerLoginPacket << (WORD)en_PACKET_C2S_GAME_REQ_LOGIN;
								*ReqGameServerLoginPacket << G_SelectCheckArray[k]->AccountId;

								int8 DummyClientNameLen = (int8)(G_SelectCheckArray[k]->LoginId.length() * 2);
								*ReqGameServerLoginPacket << DummyClientNameLen;
								ReqGameServerLoginPacket->InsertData(G_SelectCheckArray[k]->LoginId.c_str(), DummyClientNameLen);

								*ReqGameServerLoginPacket << IsDummy;

								G_SelectCheckArray[k]->SendRingBuf.Enqueue(ReqGameServerLoginPacket);
							}
							break;
							case en_PACKET_S2C_GAME_RES_LOGIN:
							{
								bool LoginSucess = false;

								*RecvPacket >> LoginSucess;

								if (LoginSucess == true)
								{
									int8 PlayerCount;
									*RecvPacket >> PlayerCount;

									if (PlayerCount > 0)
									{
										// 첫번째 캐릭으로 게임 접속 시도									
										*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectId;

										int8 CharacterNameLen;
										*RecvPacket >> CharacterNameLen;
										RecvPacket->GetData(G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectName, CharacterNameLen);

										int8 CharacterState;
										*RecvPacket >> CharacterState;

										*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectPositionInfo.PositionX;
										*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectPositionInfo.PositionY;

										int8 CharacterMoveDir;
										*RecvPacket >> CharacterMoveDir;

										*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.Level;
										*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.HP;
										*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.MaxHP;
										*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.MP;
										*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.MaxMP;
										*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.DP;
										*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.MaxDP;
										*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent;
										*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent;
										*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage;
										*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage;
										*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.MeleeAttackHitRate;
										*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.MagicDamage;
										*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.MagicHitRate;
										*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.Defence;
										*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.EvasionRate;
										*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.MeleeCriticalPoint;
										*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.MagicCriticalPoint;
										*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.Speed;


										int16 CharacterObjectType;
										*RecvPacket >> CharacterObjectType;
										G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectType = (en_GameObjectType)CharacterObjectType;

										*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.OwnerObjectId;

										int16 CharacterOwnerObjectType;
										*RecvPacket >> CharacterOwnerObjectType;
										G_SelectCheckArray[k]->MyCharacterGameObjectInfo.OwnerObjectType = (en_GameObjectType)CharacterOwnerObjectType;

										*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.PlayerSlotIndex;

										CMessage* ReqEnterGamePacket = CMessage::Alloc();
										ReqEnterGamePacket->Clear();

										*ReqEnterGamePacket << (int16)en_PACKET_C2S_GAME_ENTER;
										*ReqEnterGamePacket << G_SelectCheckArray[k]->AccountId;

										int8 DummyClientNameLen = (int8)(G_SelectCheckArray[k]->LoginId.length() * 2);
										*ReqEnterGamePacket << DummyClientNameLen;
										ReqEnterGamePacket->InsertData(G_SelectCheckArray[k]->LoginId.c_str(), DummyClientNameLen);

										G_SelectCheckArray[k]->SendRingBuf.Enqueue(ReqEnterGamePacket);
									}
									else
									{
										// 캐릭터 없으면 게임 캐릭터 생성
										CMessage* ReqCreateCharacterPacket = CMessage::Alloc();
										ReqCreateCharacterPacket->Clear();

										*ReqCreateCharacterPacket << (int16)en_PACKET_C2S_GAME_CREATE_CHARACTER;
										*ReqCreateCharacterPacket << (int16)en_GameObjectType::OBJECT_PLAYER_DUMMY;

										int8 DummyClientNameLen = (int8)(G_SelectCheckArray[k]->LoginId.length() * 2);
										*ReqCreateCharacterPacket << DummyClientNameLen;
										ReqCreateCharacterPacket->InsertData(G_SelectCheckArray[k]->LoginId.c_str(), DummyClientNameLen);

										*ReqCreateCharacterPacket << (int8)0;

										G_SelectCheckArray[k]->SendRingBuf.Enqueue(ReqCreateCharacterPacket);
									}

									G_SelectCheckArray[k]->IsLogin = true;
								}
								else
								{

								}
							}
							break;
							case en_PACKET_S2C_GAME_CREATE_CHARACTER:
							{
								bool CharacterCreateSuccess = false;

								*RecvPacket >> CharacterCreateSuccess;

								if (CharacterCreateSuccess == true)
								{
									*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectId;

									int8 CharacterNameLen;
									*RecvPacket >> CharacterNameLen;
									RecvPacket->GetData(G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectName, CharacterNameLen);

									int8 CharacterState;
									*RecvPacket >> CharacterState;

									*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectPositionInfo.PositionX;
									*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectPositionInfo.PositionY;

									int8 CharacterMoveDir;
									*RecvPacket >> CharacterMoveDir;

									*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.Level;
									*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.HP;
									*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.MaxHP;
									*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.MP;
									*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.MaxMP;
									*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.DP;
									*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.MaxDP;
									*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent;
									*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent;
									*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage;
									*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage;
									*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.MeleeAttackHitRate;
									*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.MagicDamage;
									*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.MagicHitRate;
									*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.Defence;
									*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.EvasionRate;
									*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.MeleeCriticalPoint;
									*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.MagicCriticalPoint;
									*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectStatInfo.Speed;

									int16 CharacterObjectType;
									*RecvPacket >> CharacterObjectType;
									*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.OwnerObjectId;

									int16 CharacterOwnerObjectType;
									*RecvPacket >> CharacterOwnerObjectType;
									*RecvPacket >> G_SelectCheckArray[k]->MyCharacterGameObjectInfo.PlayerSlotIndex;

									CMessage* ReqEnterGamePacket = CMessage::Alloc();
									ReqEnterGamePacket->Clear();

									*ReqEnterGamePacket << (int16)en_PACKET_C2S_GAME_ENTER;
									*ReqEnterGamePacket << G_SelectCheckArray[k]->AccountId;

									int8 DummyClientNameLen = (int8)(G_SelectCheckArray[k]->LoginId.length() * 2);
									*ReqEnterGamePacket << DummyClientNameLen;
									ReqEnterGamePacket->InsertData(G_SelectCheckArray[k]->LoginId.c_str(), DummyClientNameLen);

									G_SelectCheckArray[k]->SendRingBuf.Enqueue(ReqEnterGamePacket);
								}
							}
							break;
							case en_PACKET_S2C_GAME_ENTER:
							{
								G_SelectCheckArray[k]->IsEnterGame = true;
							}
							break;
							case en_PACKET_S2C_MESSAGE:
							{

							}
							break;
							case en_PACKET_S2C_MOVE:
							{
								int64 AccountId;
								int64 PlayerId;
								int16 ObjectType;
								int8 CreatureState;
								int32 PositionX;
								int32 PositionY;
								int8 MoveDir;

								*RecvPacket >> AccountId;
								*RecvPacket >> PlayerId;
								*RecvPacket >> ObjectType;
								*RecvPacket >> CreatureState;
								*RecvPacket >> PositionX;
								*RecvPacket >> PositionY;
								*RecvPacket >> MoveDir;

								G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectPositionInfo.State = (en_CreatureState)CreatureState;
								G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectPositionInfo.PositionX = PositionX;
								G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectPositionInfo.PositionY = PositionY;
								G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectPositionInfo.MoveDir = (en_MoveDir)MoveDir;

								CMessage* ReqMoveStopPacket = CMessage::Alloc();

								ReqMoveStopPacket->Clear();
								*ReqMoveStopPacket << (int16)en_PACKET_C2S_OBJECT_STATE_CHANGE;
								*ReqMoveStopPacket << G_SelectCheckArray[k]->AccountId;
								*ReqMoveStopPacket << G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectId;
								*ReqMoveStopPacket << (int16)G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectType;
								*ReqMoveStopPacket << G_SelectCheckArray[k]->MyCharacterGameObjectInfo.ObjectId;
								*ReqMoveStopPacket << (int16)en_StateChange::MOVE_TO_STOP;

								G_SelectCheckArray[k]->SendRingBuf.Enqueue(ReqMoveStopPacket);
							}
							break;
							case en_PACKET_S2C_PING:
								{
									CMessage* ReqPongPacket = CMessage::Alloc();
									ReqPongPacket->Clear();

									*ReqPongPacket << (int16)en_PACKET_TYPE::en_PACKET_C2S_PONG;
									G_SelectCheckArray[k]->SendRingBuf.Enqueue(ReqPongPacket);
								}
								break;
							default:
								break;
							}
						}

						RecvPacket->Free();
					}

					if (FD_ISSET(G_SelectCheckArray[k]->ClientSocket, &WriteSet) || G_SelectCheckArray[k]->SendRingBuf.GetUseSize() > 0)
					{
						while (1)
						{
							CMessage* Packet = nullptr;

							if (G_SelectCheckArray[k]->SendRingBuf.Dequeue(&Packet) == true)
							{
								Packet->Encode();

								int SendRet = send(G_SelectCheckArray[k]->ClientSocket, Packet->GetBufferPtr(), Packet->GetUseBufferSize(), 0);
								if (SendRet == SOCKET_ERROR)
								{
									DWORD Error = WSAGetLastError();
									if (Error != WSAEWOULDBLOCK)
									{
										break;
									}
								}

								Packet->Free();
							}
							else
							{
								break;
							}
						}
					}
				}
			}
			else
			{

			}
		}
	}

	return 0;
}
