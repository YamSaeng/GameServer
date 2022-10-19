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
class CToolItem;
class CConsumable;
class CMaterialItem;
class CSkill;
class CFurnace;
class CSawmill;
class CPotato;
class CCorn;
class CCropItem;
class CMapTile;

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

	// ������ 
	CMemoryPoolTLS<CItem>* _ItemMemoryPool;
	CMemoryPoolTLS<CWeaponItem>* _WeaponMemoryPool;
	CMemoryPoolTLS<CArmorItem>* _ArmorMemoryPool;
	CMemoryPoolTLS<CToolItem>* _ToolMemoryPool;
	CMemoryPoolTLS<CConsumable>* _ConsumableMemoryPool;
	CMemoryPoolTLS<CMaterialItem>* _MaterialMemoryPool;
	CMemoryPoolTLS<CArchitectureItem>* _ArchitectureMemoryPool;
	CMemoryPoolTLS<CCropItem>* _CropItemMemoryPool;

	// ������Ʈ
	CMemoryPoolTLS<CPlayer>* _PlayerMemoryPool;
	CMemoryPoolTLS<CSlime>* _SlimeMemoryPool;
	CMemoryPoolTLS<CBear>* _BearMemoryPool;
	CMemoryPoolTLS<CTree>* _TreeMemoryPool;
	CMemoryPoolTLS<CStone>* _StoneMemoryPool;	
	CMemoryPoolTLS<CFurnace>* _FurnaceMemoryPool;
	CMemoryPoolTLS<CSawmill>* _SamillMemoryPool;	
	CMemoryPoolTLS<CPotato>* _PotatoMemoryPool;
	CMemoryPoolTLS<CCorn>* _CornMemoryPool;
	CMemoryPoolTLS<CMapTile>* _MapTileMemoryPool;

	int64 _GameServerObjectId;

	CMemoryPoolTLS<CSkill>* _SkillMemoryPool;
	CMemoryPoolTLS<st_PassiveSkillInfo>* _PassiveSkillInfoMemoryPool;
	CMemoryPoolTLS<st_AttackSkillInfo>* _AttackSkillInfoMemoryPool;	
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

	//----------------------------------------
	// �÷��̾� �迭 �ε��� �ݳ�
	//----------------------------------------
	void PlayerIndexReturn(int32 PlayerIndex);

	//-----------------------------------------------------
	// ������Ʈ ����
	//-----------------------------------------------------
	CGameObject* ObjectCreate(en_GameObjectType ObjectType);
	//------------------------------------------------------------------------
	// ������Ʈ �ݳ�
	//------------------------------------------------------------------------
	void ObjectReturn(CGameObject* ReturnObject);

	//--------------------
	// ��ų ����
	//--------------------
	CSkill* SkillCreate();
	//------------------------------------
	// ��ų �ݳ�
	//------------------------------------
	void SkillReturn(CSkill* ReturnSkill);

	//----------------------------------------------------------
	// ������ ����
	//----------------------------------------------------------
	CItem* ItemCreate(en_SmallItemCategory NewItemSmallCategory);
	//---------------------------------
	// ������ �ݳ�
	//---------------------------------
	void ItemReturn(CItem* ReturnItem);

	//-----------------------------------------------
	// ��ų ���� ����
	//-----------------------------------------------
	st_SkillInfo* SkillInfoCreate(st_SkillInfo* SkillInfoData, int8 SkillLevel);
	//-----------------------------------------------
	// ��ų ���� �ݳ�
	//-----------------------------------------------
	void SkillInfoReturn(en_SkillType SkillType, st_SkillInfo* ReturnSkillInfo);

	//-----------------------------------
	// �ʿ� ������ ������Ʈ ����
	//-----------------------------------
	void MapObjectSpawn(int64& MapID);

	//-------------------------------------------
	// �ʿ� �Ҵ�Ǿ� �ִ� Ÿ�� ���� �����ͼ� ����
	//-------------------------------------------
	void MapTileInfoSpawn(int64& MapID);

	//------------------------------------------------------------------------------------------------
	// ���ӿ�����Ʈ���� ������ ����
	//------------------------------------------------------------------------------------------------
	void ObjectItemSpawn(CChannel* SpawnChannel, int64 KillerId, en_GameObjectType KillerObjectType, st_Vector2Int SpawnIntPosition, st_Vector2 SpawnPosition, en_GameObjectType SpawnItemOwnerType, en_GameObjectType ItemDataType);
	//------------------------------------------------------------------------------------------------
	// ������ ������
	//------------------------------------------------------------------------------------------------
	void ObjectItemDropToSpawn(CGameObject* DropOwnerObject, CChannel* SpawnChannel, en_SmallItemCategory DropItemType, int32 DropItemCount);		

	//-------------------------------------
	// ���� ������Ʈ Job ����
	//-------------------------------------
	st_GameObjectJob* GameObjectJobCreate();
	//-------------------------------------------------------
	// ���� ������Ʈ Job ��ȯ
	//-------------------------------------------------------
	void GameObjectJobReturn(st_GameObjectJob* GameObjectJob);
};