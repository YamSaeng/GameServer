#pragma once
#include "ChannelManager.h"
#include "Player.h"
#include "Bear.h"
#include "Slime.h"
#include "GameServer.h"
#include "Environment.h"

class CItem;
class CWeaponItem;
class CArmorItem;
class CConsumable;
class CMaterialItem;
class CSkill;
class CFurnace;
class CSawmill;
class CPotato;
class CCropItem;

class CObjectManager
{
private:
	enum en_ObjectCount
	{
		PLAYER_MAX = 15000,		
		ITEM_MAX = 15000,
		ENVIRONMENT_MAX = 15000,
		CRAFTINGTABLE_MAX = 15000
	};	

	// 아이템 
	CMemoryPoolTLS<CItem>* _ItemMemoryPool;
	CMemoryPoolTLS<CWeaponItem>* _WeaponMemoryPool;
	CMemoryPoolTLS<CArmorItem>* _ArmorMemoryPool;
	CMemoryPoolTLS<CConsumable>* _ConsumableMemoryPool;
	CMemoryPoolTLS<CMaterialItem>* _MaterialMemoryPool;
	CMemoryPoolTLS<CArchitectureItem>* _ArchitectureMemoryPool;
	CMemoryPoolTLS<CCropItem>* _CropMemoryPool;

	// 오브젝트
	CMemoryPoolTLS<CPlayer>* _PlayerMemoryPool;
	CMemoryPoolTLS<CSlime>* _SlimeMemoryPool;
	CMemoryPoolTLS<CBear>* _BearMemoryPool;
	CMemoryPoolTLS<CTree>* _TreeMemoryPool;
	CMemoryPoolTLS<CStone>* _StoneMemoryPool;	
	CMemoryPoolTLS<CFurnace>* _FurnaceMemoryPool;
	CMemoryPoolTLS<CSawmill>* _SamillMemoryPool;
	CMemoryPoolTLS<CPotato>* _PotatoMemoryPool;

	int64 _GameServerObjectId;

	CMemoryPoolTLS<CSkill>* _SkillMemoryPool;
	CMemoryPoolTLS<st_AttackSkillInfo>* _AttackSkillInfoMemoryPool;
	CMemoryPoolTLS<st_TacTicSkillInfo>* _TacTicSkillInfoMemoryPool;
	CMemoryPoolTLS<st_HealSkillInfo>* _HealSkillInfoMemoryPool;
	CMemoryPoolTLS<st_BufSkillInfo>* _BufSkillInfoMemoryPool;

	CMemoryPoolTLS<st_GameObjectJob>* _GameObjectJobMemoryPool;
public:
	CGameServer* GameServer;

	CPlayer* _PlayersArray[PLAYER_MAX];	
	CItem* _ItemsArray[ITEM_MAX];
	CEnvironment* _EnvironmentsArray[ENVIRONMENT_MAX];
	CCraftingTable* _CraftingTablesArray[CRAFTINGTABLE_MAX];

	CLockFreeStack<int32> _PlayersArrayIndexs;	
	CLockFreeStack<int32> _ItemsArrayIndexs;
	CLockFreeStack<int32> _EnvironmentsArrayIndexs;
	CLockFreeStack<int32> _CraftingTableArrayIndexs;	

	CObjectManager();
	~CObjectManager();
	//-----------------------------------------------------------
	// Object를 게임에 입장시켜준다. ( 입장할 맵의 아이디를 받는다 )
	//-----------------------------------------------------------
	void ObjectEnterGame(CGameObject* EnterGameObject, int64 MapID);
	//-----------------------------------------------------------
	// Object를 게임에서 퇴장시켜준다. ( 퇴장할 맵의 아이디를 받는다 )
	//-----------------------------------------------------------
	bool ObjectLeaveGame(CGameObject* LeaveGameObject, int32 ObjectIndex, int32 _ChannelId, bool IsObjectReturn = true);

	//----------------------------------------
	// 플레이어 배열 인덱스 반납
	//----------------------------------------
	void PlayerIndexReturn(int32 PlayerIndex);

	//-----------------------------------------------------
	// 오브젝트 생성
	//-----------------------------------------------------
	CGameObject* ObjectCreate(en_GameObjectType ObjectType);
	//------------------------------------------------------------------------
	// 오브젝트 반납
	//------------------------------------------------------------------------
	void ObjectReturn(en_GameObjectType ObjectType, CGameObject* ReturnObject);

	//--------------------
	// 스킬 생성
	//--------------------
	CSkill* SkillCreate();
	//------------------------------------
	// 스킬 반납
	//------------------------------------
	void SkillReturn(CSkill* ReturnSkill);

	//----------------------------------------------------------
	// 아이템 생성
	//----------------------------------------------------------
	CItem* ItemCreate(en_SmallItemCategory NewItemSmallCategory);
	//---------------------------------
	// 아이템 반납
	//---------------------------------
	void ItemReturn(CItem* ReturnItem);

	//-----------------------------------------------
	// 스킬 정보 생성
	//-----------------------------------------------
	st_SkillInfo* SkillInfoCreate(en_SkillMediumCategory SkillMediumCategory);
	//-----------------------------------------------
	// 스킬 정보 반납
	//-----------------------------------------------
	void SkillInfoReturn(en_SkillMediumCategory SkillMediumCategory, st_SkillInfo* ReturnSkillInfo);

	//-----------------------------------
	// 맵에 설정한 오브젝트 스폰
	//-----------------------------------
	void MapObjectSpawn(int64& MapID);

	//------------------------------------------------------------------------------------------------
	// 게임오브젝트에서 아이템 스폰
	//------------------------------------------------------------------------------------------------
	void ObjectItemSpawn(int64 KillerId, en_GameObjectType KillerObjectType, st_Vector2Int SpawnPosition, en_GameObjectType SpawnItemOwnerType, en_GameObjectType ItemDataType);
	//------------------------------------------------------------------------------------------------
	// 아이템 버리기
	//------------------------------------------------------------------------------------------------
	void ObjectItemDropToSpawn(en_SmallItemCategory DropItemType, int32 DropItemCount, st_Vector2Int SpawnPosition);

	
	//----------------------------------------------------------------------------
	// 오브젝트 스폰
	//----------------------------------------------------------------------------
	void ObjectSpawn(en_GameObjectType ObjectType, st_Vector2Int SpawnPosition);

	//-------------------------------------
	// 게임 오브젝트 Job 생성
	//-------------------------------------
	st_GameObjectJob* GameObjectJobCreate();
	//-------------------------------------------------------
	// 게임 오브젝트 Job 반환
	//-------------------------------------------------------
	void GameObjectJobReturn(st_GameObjectJob* GameObjectJob);
};