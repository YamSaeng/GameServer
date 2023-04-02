#pragma once
#include "ChannelManager.h"
#include "Player.h"
#include "Goblin.h"
#include "Environment.h"

class CItem;
class CWeaponItem;
class CArmorItem;
class CToolItem;
class CConsumable;
class CMaterialItem;
class CSkill;
class CFurnace;
class CSawmill;
class CPotato;
class CCorn;
class CCropItem;
class CGeneralMerchantNPC;
class CWall;
class CRectCollision;

class CObjectManager
{
private:
	enum en_ObjectCount
	{
		OBJECT_MANAGER_PLAYER_MAX = 15000,		
		OBJECT_MANAGER_ITEM_MAX = 15000,
		OBJECT_MANAGER_ENVIRONMENT_MAX = 15000,
		OBJECT_MANAGER_CRAFTINGTABLE_MAX = 15000
	};	

	// 메모리 풀 
	CMemoryPoolTLS<CItem>* _ItemMemoryPool;
	CMemoryPoolTLS<CWeaponItem>* _WeaponMemoryPool;
	CMemoryPoolTLS<CArmorItem>* _ArmorMemoryPool;
	CMemoryPoolTLS<CToolItem>* _ToolMemoryPool;
	CMemoryPoolTLS<CConsumable>* _ConsumableMemoryPool;
	CMemoryPoolTLS<CMaterialItem>* _MaterialMemoryPool;
	CMemoryPoolTLS<CArchitectureItem>* _ArchitectureMemoryPool;
	CMemoryPoolTLS<CCropItem>* _CropItemMemoryPool;

	CMemoryPoolTLS<CRectCollision>* _RectCollisionMemoryPool;

	CMemoryPoolTLS<CWall>* _WallMemoryPool;
	CMemoryPoolTLS<CPlayer>* _PlayerMemoryPool;	
	CMemoryPoolTLS<CGeneralMerchantNPC>* _GeneralMerchantNPCMemoryPool;
	CMemoryPoolTLS<CGoblin>* _GoblinMemoryPool;	
	CMemoryPoolTLS<CTree>* _TreeMemoryPool;
	CMemoryPoolTLS<CStone>* _StoneMemoryPool;	
	CMemoryPoolTLS<CFurnace>* _FurnaceMemoryPool;
	CMemoryPoolTLS<CSawmill>* _SamillMemoryPool;	
	CMemoryPoolTLS<CPotato>* _PotatoMemoryPool;
	CMemoryPoolTLS<CCorn>* _CornMemoryPool;	

	// 오브젝트에 부여할 고유 ID값
	int64 _GameServerObjectId;

	CMemoryPoolTLS<CSkill>* _SkillMemoryPool;
	CMemoryPoolTLS<st_SkillInfo>* _SkillInfoMemoryPool;	

	CMemoryPoolTLS<st_GameObjectJob>* _GameObjectJobMemoryPool;
public:
	array<CPlayer*, static_cast<size_t>(en_ObjectCount::OBJECT_MANAGER_PLAYER_MAX)> _PlayersArray = { nullptr };
	array<CItem*, static_cast<size_t>(en_ObjectCount::OBJECT_MANAGER_ITEM_MAX)> _ItemsArray = { nullptr };
	array<CEnvironment*, static_cast<size_t>(en_ObjectCount::OBJECT_MANAGER_ENVIRONMENT_MAX)> _EnvironmentsArray = { nullptr };
	array<CCraftingTable*, static_cast<size_t>(en_ObjectCount::OBJECT_MANAGER_CRAFTINGTABLE_MAX)> _CraftingTablesArray = { nullptr };	

	CLockFreeStack<int32> _PlayersArrayIndexs;	
	CLockFreeStack<int32> _ItemsArrayIndexs;
	CLockFreeStack<int32> _EnvironmentsArrayIndexs;
	CLockFreeStack<int32> _CraftingTableArrayIndexs;	

	CObjectManager();
	~CObjectManager();
		
	// 플레이어 배열 인덱스 반납	
	void PlayerIndexReturn(int32 PlayerIndex);
		
	// 오브젝트 생성	
	CGameObject* ObjectCreate(en_GameObjectType ObjectType);	
	// 오브젝트 반납	
	void ObjectReturn(CGameObject* ReturnObject);

	// 사각 충돌체 생성
	CRectCollision* RectCollisionCreate();
	// 사각 충돌체 반납
	void RectCollisionReturn(CRectCollision* RectCollision);

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
	st_SkillInfo* SkillInfoCreate(en_SkillType SkillType, int8 SkillLevel = 1);
	//-----------------------------------------------
	// 스킬 정보 반납
	//-----------------------------------------------
	void SkillInfoReturn(en_SkillType SkillType, st_SkillInfo* ReturnSkillInfo);		

	//-------------------------------------------
	// 맵에 할당되어 있는 타일 정보 가져와서 스폰
	//-------------------------------------------
	void MapTileInfoSpawn(int64& MapID);

	//------------------------------------------------------------------------------------------------
	// 게임오브젝트에서 아이템 스폰
	//------------------------------------------------------------------------------------------------
	void ObjectItemSpawn(CChannel* SpawnChannel, int64 KillerId, en_GameObjectType KillerObjectType, st_Vector2Int SpawnIntPosition,
		st_Vector2 SpawnPosition, en_GameObjectType SpawnItemOwnerType);
	//------------------------------------------------------------------------------------------------
	// 아이템 버리기
	//------------------------------------------------------------------------------------------------
	void ObjectItemDropToSpawn(CGameObject* DropOwnerObject, CChannel* SpawnChannel, en_SmallItemCategory DropItemType, int32 DropItemCount);		

	//-------------------------------------
	// 게임 오브젝트 Job 생성
	//-------------------------------------
	st_GameObjectJob* GameObjectJobCreate();
	//-------------------------------------------------------
	// 게임 오브젝트 Job 반환
	//-------------------------------------------------------
	void GameObjectJobReturn(st_GameObjectJob* GameObjectJob);
};