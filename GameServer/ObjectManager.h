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

	// �޸� Ǯ 
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

	// ������Ʈ�� �ο��� ���� ID��
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
		
	// �÷��̾� �迭 �ε��� �ݳ�	
	void PlayerIndexReturn(int32 PlayerIndex);
		
	// ������Ʈ ����	
	CGameObject* ObjectCreate(en_GameObjectType ObjectType);	
	// ������Ʈ �ݳ�	
	void ObjectReturn(CGameObject* ReturnObject);

	// �簢 �浹ü ����
	CRectCollision* RectCollisionCreate();
	// �簢 �浹ü �ݳ�
	void RectCollisionReturn(CRectCollision* RectCollision);

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
	st_SkillInfo* SkillInfoCreate(en_SkillType SkillType, int8 SkillLevel = 1);
	//-----------------------------------------------
	// ��ų ���� �ݳ�
	//-----------------------------------------------
	void SkillInfoReturn(en_SkillType SkillType, st_SkillInfo* ReturnSkillInfo);		

	//-------------------------------------------
	// �ʿ� �Ҵ�Ǿ� �ִ� Ÿ�� ���� �����ͼ� ����
	//-------------------------------------------
	void MapTileInfoSpawn(int64& MapID);

	//------------------------------------------------------------------------------------------------
	// ���ӿ�����Ʈ���� ������ ����
	//------------------------------------------------------------------------------------------------
	void ObjectItemSpawn(CChannel* SpawnChannel, int64 KillerId, en_GameObjectType KillerObjectType, st_Vector2Int SpawnIntPosition,
		st_Vector2 SpawnPosition, en_GameObjectType SpawnItemOwnerType);
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