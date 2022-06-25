#include "pch.h"
#include "ObjectManager.h"
#include "DataManager.h"
#include "GameServerMessage.h"
#include "Item.h"
#include "Skill.h"
#include "MapManager.h"
#include "CraftingTable.h"
#include "Furnace.h"
#include "Sawmill.h"
#include <atlbase.h>

CObjectManager::CObjectManager()
{
	_PlayerMemoryPool = new CMemoryPoolTLS<CPlayer>();
	_SlimeMemoryPool = new CMemoryPoolTLS<CSlime>();
	_BearMemoryPool = new CMemoryPoolTLS<CBear>();

	_ItemMemoryPool = new CMemoryPoolTLS<CItem>();
	_WeaponMemoryPool = new CMemoryPoolTLS<CWeapon>();
	_ArmorMemoryPool = new CMemoryPoolTLS<CArmor>();
	_MaterialMemoryPool = new CMemoryPoolTLS<CMaterial>();
	_ArchitectureMemoryPool = new CMemoryPoolTLS<CArchitecture>();
	_ConsumableMemoryPool = new CMemoryPoolTLS<CConsumable>();

	_TreeMemoryPool = new CMemoryPoolTLS<CTree>();
	_StoneMemoryPool = new CMemoryPoolTLS<CStone>();
	_FurnaceMemoryPool = new CMemoryPoolTLS<CFurnace>();
	_SamillMemoryPool = new CMemoryPoolTLS<CSawmill>();

	_SkillMemoryPool = new CMemoryPoolTLS<CSkill>();

	_AttackSkillInfoMemoryPool = new CMemoryPoolTLS<st_AttackSkillInfo>();
	_TacTicSkillInfoMemoryPool = new CMemoryPoolTLS<st_TacTicSkillInfo>();
	_HealSkillInfoMemoryPool = new CMemoryPoolTLS<st_HealSkillInfo>();
	_BufSkillInfoMemoryPool = new CMemoryPoolTLS<st_BufSkillInfo>();

	_GameObjectJobMemoryPool = new CMemoryPoolTLS<st_GameObjectJob>();

	_GameServerObjectId = 10000;

	// 오브젝트 매니저가 소유중인 플레이어, 몬스터, 아이템 미리 할당해서 보관
	for (int PlayerCount = PLAYER_MAX - 1; PlayerCount >= 0; --PlayerCount)
	{
		_PlayersArray[PlayerCount] = (CPlayer*)ObjectCreate(en_GameObjectType::OBJECT_PLAYER);
		_PlayersArrayIndexs.Push(PlayerCount);
	}

	for (int MonsterCount = MONSTER_MAX - 1; MonsterCount >= 0; --MonsterCount)
	{
		_MonstersArray[MonsterCount] = nullptr;
		_MonstersArrayIndexs.Push(MonsterCount);
	}

	for (int ItemCount = ITEM_MAX - 1; ItemCount >= 0; --ItemCount)
	{
		_ItemsArray[ItemCount] = nullptr;
		_ItemsArrayIndexs.Push(ItemCount);
	}

	for (int EnvironmentCount = ENVIRONMENT_MAX - 1; EnvironmentCount >= 0; --EnvironmentCount)
	{
		_EnvironmentsArray[EnvironmentCount] = nullptr;
		_EnvironmentsArrayIndexs.Push(EnvironmentCount);
	}

	for (int CraftingTableCount = CRAFTINGTABLE_MAX - 1; CraftingTableCount >= 0; --CraftingTableCount)
	{
		_CraftingTablesArray[CraftingTableCount] = nullptr;
		_CraftingTableArrayIndexs.Push(CraftingTableCount);
	}
}

CObjectManager::~CObjectManager()
{
	delete _PlayerMemoryPool;
	delete _SlimeMemoryPool;
	delete _BearMemoryPool;
	delete _WeaponMemoryPool;
	delete _MaterialMemoryPool;
	delete _ConsumableMemoryPool;
	delete _TreeMemoryPool;
	delete _StoneMemoryPool;
}

void CObjectManager::ObjectEnterGame(CGameObject* EnterGameObject, int64 MapID)
{
	bool IsEnterChannel = true;

	// 채널 찾는다.
	CMap* Map = G_MapManager->GetMap(MapID);
	if (Map == nullptr)
	{
		CRASH("ObjectManager Add EnterChannel이 nullptr");
	}

	switch (EnterGameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
	{
		CPlayer* Player = (CPlayer*)EnterGameObject;

		Player->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::SPAWN_IDLE;
	
		Map->GetChannelManager()->Find(1)->EnterChannel(EnterGameObject);
	}
	break;
	case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
	case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
	case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
	case en_GameObjectType::OBJECT_THIEF_PLAYER:
	case en_GameObjectType::OBJECT_ARCHER_PLAYER:
	{
		CPlayer* Player = (CPlayer*)EnterGameObject;

		Player->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::SPAWN_IDLE;

		// 채널 입장
		Map->GetChannelManager()->Find(1)->EnterChannel(EnterGameObject, &Player->_SpawnPosition);
	}
	break;
	case en_GameObjectType::OBJECT_SLIME:
	case en_GameObjectType::OBJECT_BEAR:
	{
		CMonster* Monster = (CMonster*)EnterGameObject;

		// 인덱스 가져오기
		_MonstersArrayIndexs.Pop(&EnterGameObject->_ObjectManagerArrayIndex);
		// 배열에 저장
		_MonstersArray[EnterGameObject->_ObjectManagerArrayIndex] = Monster;

		// 몬스터 주위 오브젝트 정보 저장
		Monster->_FieldOfViewPlayers = Map->GetFieldOfViewPlayer(Monster, Monster->_FieldOfViewDistance);
		
		// 채널 입장
		Map->GetChannelManager()->Find(1)->EnterChannel(EnterGameObject, &Monster->_SpawnPosition);		

		// 몬스터 추가하면 몬스터 주위 플레이어들에게 몬스터를 소환하라고 알림
		CMessage* ResSpawnPacket = GameServer->MakePacketResObjectSpawn(Monster);
		GameServer->SendPacketFieldOfView(Monster, ResSpawnPacket);
		ResSpawnPacket->Free();

		// 몬스터 소환할때 소환 이펙트 출력
		CMessage* ResEffectPacket = GameServer->MakePacketEffect(Monster->_GameObjectInfo.ObjectId, en_EffectType::EFFECT_OBJECT_SPAWN, 0.5f);
		GameServer->SendPacketFieldOfView(Monster, ResEffectPacket);
		ResEffectPacket->Free();
	}
	break;
	case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_SWORD:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_WOOD_ARMOR:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_HELMET:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_BOOT:
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_SKILL_BOOK:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIME_GEL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_BRONZE_COIN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_LEATHER:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_LOG:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_STONE:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_FLANK:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_YARN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_CHAR_COAL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_COPPER_NUGGET:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_COPPER_INGOT:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_IRON_NUGGET:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_IRON_INGOT:
		{
			CItem* Item = (CItem*)EnterGameObject;

			IsEnterChannel = Map->GetChannelManager()->Find(1)->EnterChannel(EnterGameObject, &Item->_SpawnPosition);
			if (IsEnterChannel == true)
			{
				// 중복되지 않은 아이템 스폰
				// 인덱스 가져오기	
				_ItemsArrayIndexs.Pop(&EnterGameObject->_ObjectManagerArrayIndex);
				// 배열에 저장
				_ItemsArray[EnterGameObject->_ObjectManagerArrayIndex] = Item;			

				Item->SetDestoryTime(1800);
				Item->ItemSetTarget(Item->_GameObjectInfo.OwnerObjectType, Item->_GameObjectInfo.OwnerObjectId);

				CMessage* ResSpawnPacket = GameServer->MakePacketResObjectSpawn(Item);
				GameServer->SendPacketFieldOfView(Item, ResSpawnPacket);
				ResSpawnPacket->Free();
			}
			else
			{
				// 중복된 아이템의 경우 메모리에 반납
				ObjectReturn(Item->_GameObjectInfo.ObjectType, Item);
			}
		}
		break;
	case en_GameObjectType::OBJECT_STONE:
	case en_GameObjectType::OBJECT_TREE:
		{
			CEnvironment* Environment = (CEnvironment*)EnterGameObject;

			// 인덱스 가져오기
			_EnvironmentsArrayIndexs.Pop(&EnterGameObject->_ObjectManagerArrayIndex);
			// 배열에 저장
			_EnvironmentsArray[EnterGameObject->_ObjectManagerArrayIndex] = Environment;

			Map->GetChannelManager()->Find(1)->EnterChannel(EnterGameObject, &Environment->_SpawnPosition);		

			CMessage* ResSpawnPacket = GameServer->MakePacketResObjectSpawn(Environment);
			GameServer->SendPacketFieldOfView(Environment, ResSpawnPacket);
			ResSpawnPacket->Free();
		}
		break;
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL:
		{
			CCraftingTable* CraftingTable = (CCraftingTable*)EnterGameObject;

			_CraftingTableArrayIndexs.Pop(&EnterGameObject->_ObjectManagerArrayIndex);
			_CraftingTablesArray[EnterGameObject->_ObjectManagerArrayIndex] = CraftingTable;

			Map->GetChannelManager()->Find(1)->EnterChannel(EnterGameObject, &CraftingTable->_SpawnPosition);

			CMessage* ResSpawnPacket = GameServer->MakePacketResObjectSpawn(CraftingTable);
			GameServer->SendPacketFieldOfView(CraftingTable, ResSpawnPacket);
			ResSpawnPacket->Free();
		}
		break;
	}
}

bool CObjectManager::ObjectLeaveGame(CGameObject* LeaveGameObject, int32 ObjectIndex, int32 _ChannelId, bool IsObjectReturn)
{
	bool RemoveSuccess = false;

	// 타입에 따라 관리당하고 있는 자료구조에서 자신을 삭제
	// 채널에서 삭제	
	switch (LeaveGameObject->_GameObjectInfo.ObjectType)
	{	
	case en_GameObjectType::OBJECT_SLIME:
	case en_GameObjectType::OBJECT_BEAR:
		LeaveGameObject->GetChannel()->LeaveChannel(LeaveGameObject);

		_MonstersArrayIndexs.Push(ObjectIndex);
		break;
	case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_SWORD:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_WOOD_ARMOR:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_HELMET:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_BOOT:
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_SKILL_BOOK:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIME_GEL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_BRONZE_COIN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_LEATHER:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_LOG:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_STONE:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_FLANK:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_YARN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_CHAR_COAL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_COPPER_NUGGET:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_COPPER_INGOT:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_IRON_NUGGET:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_IRON_INGOT:
		LeaveGameObject->GetChannel()->LeaveChannel(LeaveGameObject);

		_ItemsArrayIndexs.Push(ObjectIndex);
		break;
	case en_GameObjectType::OBJECT_STONE:
	case en_GameObjectType::OBJECT_TREE:
		LeaveGameObject->GetChannel()->LeaveChannel(LeaveGameObject);

		_EnvironmentsArrayIndexs.Push(ObjectIndex);
		break;
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL:
		LeaveGameObject->GetChannel()->LeaveChannel(LeaveGameObject);

		_CraftingTableArrayIndexs.Push(ObjectIndex);
		break;
	}

	if (IsObjectReturn == true)
	{
		// 오브젝트 메모리 풀에 반환
		ObjectReturn(LeaveGameObject->_GameObjectInfo.ObjectType, LeaveGameObject);
	}

	return RemoveSuccess;
}

void CObjectManager::PlayerIndexReturn(int32 PlayerIndex)
{
	_PlayersArrayIndexs.Push(PlayerIndex);
}

CGameObject* CObjectManager::ObjectCreate(en_GameObjectType ObjectType)
{
	CGameObject* NewObject = nullptr;

	switch (ObjectType)
	{
	case en_GameObjectType::OBJECT_PLAYER:
		NewObject = _PlayerMemoryPool->Alloc();
		break;
	case en_GameObjectType::OBJECT_SLIME:
		NewObject = _SlimeMemoryPool->Alloc();
		break;
	case en_GameObjectType::OBJECT_BEAR:
		NewObject = _BearMemoryPool->Alloc();
		break;	
	case en_GameObjectType::OBJECT_STONE:
		NewObject = _StoneMemoryPool->Alloc();
		break;
	case en_GameObjectType::OBJECT_TREE:
		NewObject = _TreeMemoryPool->Alloc();
		break;
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
		NewObject = _FurnaceMemoryPool->Alloc();
		break;
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL:
		NewObject = _SamillMemoryPool->Alloc();
		break;
	}

	return NewObject;
}

void CObjectManager::ObjectReturn(en_GameObjectType ObjectType, CGameObject* ReturnObject)
{
	switch (ObjectType)
	{
	case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
	case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
	case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
	case en_GameObjectType::OBJECT_THIEF_PLAYER:
	case en_GameObjectType::OBJECT_ARCHER_PLAYER:
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
		_PlayerMemoryPool->Free((CPlayer*)ReturnObject);
		break;
	case en_GameObjectType::OBJECT_SLIME:
		_SlimeMemoryPool->Free((CSlime*)ReturnObject);
		break;
	case en_GameObjectType::OBJECT_BEAR:
		_BearMemoryPool->Free((CBear*)ReturnObject);
		break;	
	case en_GameObjectType::OBJECT_STONE:
		_StoneMemoryPool->Free((CStone*)ReturnObject);
		break;
	case en_GameObjectType::OBJECT_TREE:
		_TreeMemoryPool->Free((CTree*)ReturnObject);
		break;
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
		_FurnaceMemoryPool->Free((CFurnace*)ReturnObject);
		break;
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL:		
		_SamillMemoryPool->Free((CSawmill*)ReturnObject);
		break;
	}
}

CSkill* CObjectManager::SkillCreate()
{
	return _SkillMemoryPool->Alloc();
}

void CObjectManager::SkillReturn(CSkill* ReturnSkill)
{
	_SkillMemoryPool->Free(ReturnSkill);
}

CItem* CObjectManager::ItemCreate(en_SmallItemCategory NewItemSmallCategory)
{
	CItem* NewItem = nullptr;

	switch (NewItemSmallCategory)
	{
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD:
		NewItem = _WeaponMemoryPool->Alloc();
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_WOOD:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER:
		NewItem = _ArmorMemoryPool->Alloc();
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_HEAL_SMALL:
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_LEATHER:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIMEGEL:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_BRONZE_COIN:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIVER_COIN:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_GOLD_COIN:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_STONE:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_YARN:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_CHAR_COAL:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_COPPER_NUGGET:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_COPPER_INGOT:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_IRON_NUGGET:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_IRON_INGOT:
		NewItem = _MaterialMemoryPool->Alloc();
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_CRAFTING_TABLE_FURANCE:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_CRAFTING_TABLE_SAWMILL:
		NewItem = _ArchitectureMemoryPool->Alloc();
		break;
	}

	if (NewItem != nullptr)
	{
		NewItem->_GameObjectInfo.ObjectId = InterlockedIncrement64(&_GameServerObjectId);
	}

	return NewItem;
}

void CObjectManager::ItemReturn(CItem* ReturnItem)
{
	switch (ReturnItem->_ItemInfo.ItemSmallCategory)
	{
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD:
		_WeaponMemoryPool->Free((CWeapon*)ReturnItem);
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_WOOD:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER:
		_ArmorMemoryPool->Free((CArmor*)ReturnItem);
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_HEAL_SMALL:
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_LEATHER:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIMEGEL:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_BRONZE_COIN:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIVER_COIN:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_GOLD_COIN:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_STONE:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_YARN:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_CHAR_COAL:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_COPPER_NUGGET:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_COPPER_INGOT:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_IRON_NUGGET:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_IRON_INGOT:
		_MaterialMemoryPool->Free((CMaterial*)ReturnItem);
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_CRAFTING_TABLE_FURANCE:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_CRAFTING_TABLE_SAWMILL:
		_ArchitectureMemoryPool->Free((CArchitecture*)ReturnItem);
		break;
	}
}

st_SkillInfo* CObjectManager::SkillInfoCreate(en_SkillMediumCategory SkillMediumCategory)
{
	switch (SkillMediumCategory)
	{
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_ATTACK:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_ATTACK:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_ATTACK:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_ATTACK:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_THIEF_ATTACK:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ARCHER_ATTACK:
		return _AttackSkillInfoMemoryPool->Alloc();
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_TACTIC:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_TACTIC:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_TACTIC:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_TACTIC:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_THIEF_TACTIC:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ARCHER_TACTIC:
		return _TacTicSkillInfoMemoryPool->Alloc();
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_HEAL:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_HEAL:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_HEAL:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_HEAL:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_THIEF_HEAL:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ARCHER_HEAL:
		return _HealSkillInfoMemoryPool->Alloc();
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_BUF:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_BUF:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_BUF:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_BUF:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_THIEF_BUF:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ARCHER_BUF:
		return _BufSkillInfoMemoryPool->Alloc();
	}
}

void CObjectManager::SkillInfoReturn(en_SkillMediumCategory SkillMediumCategory, st_SkillInfo* ReturnSkillInfo)
{
	switch (SkillMediumCategory)
	{
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_ATTACK:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_ATTACK:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_ATTACK:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_ATTACK:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_THIEF_ATTACK:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ARCHER_ATTACK:
		_AttackSkillInfoMemoryPool->Free((st_AttackSkillInfo*)ReturnSkillInfo);
		break;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_TACTIC:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_TACTIC:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_TACTIC:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_TACTIC:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_THIEF_TACTIC:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ARCHER_TACTIC:
		_TacTicSkillInfoMemoryPool->Free((st_TacTicSkillInfo*)ReturnSkillInfo);
		return;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_HEAL:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_HEAL:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_HEAL:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_HEAL:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_THIEF_HEAL:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ARCHER_HEAL:
		_HealSkillInfoMemoryPool->Free((st_HealSkillInfo*)ReturnSkillInfo);
		break;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_BUF:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_BUF:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_BUF:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_BUF:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_THIEF_BUF:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ARCHER_BUF:
		_BufSkillInfoMemoryPool->Free((st_BufSkillInfo*)ReturnSkillInfo);
		break;
	}
}

void CObjectManager::MapObjectSpawn(int64& MapID)
{
	CMap* Map = G_MapManager->GetMap(MapID);

	int32 SizeX = Map->_SizeX;
	int32 SizeY = Map->_SizeY;

	for (int Y = 0; Y < SizeY; Y++)
	{
		for (int X = 0; X < SizeX; X++)
		{
			CGameObject* NewObject = nullptr;

			switch (Map->_CollisionMapInfos[Y][X])
			{
			case en_TileMapEnvironment::TILE_MAP_TREE:
				NewObject = (CTree*)ObjectCreate(en_GameObjectType::OBJECT_TREE);
				break;
			case en_TileMapEnvironment::TILE_MAP_STONE:
				NewObject = (CTree*)ObjectCreate(en_GameObjectType::OBJECT_STONE);
				break;
			case en_TileMapEnvironment::TILE_MAP_SLIME:
				NewObject = (CSlime*)ObjectCreate(en_GameObjectType::OBJECT_SLIME);
				break;
			case en_TileMapEnvironment::TILE_MAP_BEAR:
				NewObject = (CBear*)ObjectCreate(en_GameObjectType::OBJECT_BEAR);
				break;			
			case en_TileMapEnvironment::TILE_MAP_FURNACE:
				NewObject = (CFurnace*)ObjectCreate(en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE);
				break;
			case en_TileMapEnvironment::TILE_MAP_SAMILL:
				NewObject = (CSawmill*)ObjectCreate(en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL);
				break;
			}

			if (NewObject != nullptr)
			{
				int SpawnPositionX = X + Map->_Left;
				int SpawnPositionY = Map->_Down - Y;

				st_Vector2Int NewPosition;
				NewPosition._Y = SpawnPositionY;
				NewPosition._X = SpawnPositionX;

				NewObject->_GameObjectInfo.ObjectId = InterlockedIncrement64(&_GameServerObjectId);
				NewObject->_SpawnPosition = NewPosition;
				NewObject->_NetworkState = en_ObjectNetworkState::LIVE;

				ObjectEnterGame(NewObject, Map->_MapID);
			}
		}
	}
}

void CObjectManager::ObjectItemSpawn(int64 KillerId, en_GameObjectType KillerObjectType, st_Vector2Int SpawnPosition, en_GameObjectType SpawnItemOwnerType, en_ObjectDataType MonsterDataType)
{
	bool Find = false;
	st_ItemData DropItemData;

	random_device RD;
	mt19937 Gen(RD());
	uniform_real_distribution<float> RandomDropPoint(0, 1); // 0.0 ~ 1.0
	float RandomPoint = 100 * RandomDropPoint(Gen);

	int32 Sum = 0;

	switch ((en_GameObjectType)SpawnItemOwnerType)
	{
	case en_GameObjectType::OBJECT_SLIME:
	case en_GameObjectType::OBJECT_BEAR:
	{
		auto FindMonsterDropItem = G_Datamanager->_Monsters.find(MonsterDataType);
		st_MonsterData MonsterData = *(*FindMonsterDropItem).second;

		for (st_DropData DropItem : MonsterData.DropItems)
		{
			Sum += DropItem.Probability;

			if (Sum >= RandomPoint)
			{
				Find = true;
				// 드랍 확정 되면 해당 아이템 읽어오기
				auto FindDropItemInfo = G_Datamanager->_Items.find((int16)DropItem.DropItemSmallCategory);
				if (FindDropItemInfo == G_Datamanager->_Items.end())
				{
					CRASH("DropItemInfo를 찾지 못함");
				}

				DropItemData = *(*FindDropItemInfo).second;

				uniform_int_distribution<int> RandomDropItemCount(DropItem.MinCount, DropItem.MaxCount);
				DropItemData.ItemCount = RandomDropItemCount(Gen);
				DropItemData.SmallItemCategory = DropItem.DropItemSmallCategory;
				break;
			}
		}
	}
	break;
	case en_GameObjectType::OBJECT_STONE:
	case en_GameObjectType::OBJECT_TREE:
	{
		auto FindEnvironmentDropItem = G_Datamanager->_Environments.find(MonsterDataType);
		st_EnvironmentData EnvironmentData = *(*FindEnvironmentDropItem).second;

		for (st_DropData DropItem : EnvironmentData.DropItems)
		{
			Sum += DropItem.Probability;

			if (Sum >= RandomPoint)
			{
				Find = true;
				// 드랍 확정 되면 해당 아이템 읽어오기
				auto FindDropItemInfo = G_Datamanager->_Items.find((int16)DropItem.DropItemSmallCategory);
				if (FindDropItemInfo == G_Datamanager->_Items.end())
				{
					CRASH("DropItemInfo를 찾지 못함");
				}

				DropItemData = *(*FindDropItemInfo).second;

				uniform_int_distribution<int> RandomDropItemCount(DropItem.MinCount, DropItem.MaxCount);
				DropItemData.ItemCount = RandomDropItemCount(Gen);
				DropItemData.SmallItemCategory = DropItem.DropItemSmallCategory;
				break;
			}
		}
	}
	break;
	}

	if (Find == true)
	{
		bool ItemIsQuickSlotUse = false;
		bool ItemRotated = false;
		int16 ItemWidth = 0;
		int16 ItemHeight = 0;
		int8 ItemLargeCategory = 0;
		int8 ItemMediumCategory = 0;
		int16 ItemSmallCategory = 0;
		wstring ItemName;
		int16 ItemCount = 0;
		wstring ItemThumbnailImagePath;
		bool ItemEquipped = false;
		int16 ItemTilePositionX = 0;
		int16 ItemTilePositionY = 0;
		int32 ItemMinDamage = 0;
		int32 ItemMaxDamage = 0;
		int32 ItemDefence = 0;
		int32 ItemMaxCount = 0;

		switch (DropItemData.SmallItemCategory)
		{
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD:
		{
			ItemIsQuickSlotUse = false;
			ItemLargeCategory = (int8)DropItemData.LargeItemCategory;
			ItemMediumCategory = (int8)DropItemData.MediumItemCategory;
			ItemSmallCategory = (int16)DropItemData.SmallItemCategory;
			ItemName = (LPWSTR)CA2W(DropItemData.ItemName.c_str());
			ItemCount = DropItemData.ItemCount;
			ItemEquipped = false;
			ItemThumbnailImagePath = (LPWSTR)CA2W(DropItemData.ItemThumbnailImagePath.c_str());

			st_ItemData* WeaponItemData = (*G_Datamanager->_Items.find((int16)DropItemData.SmallItemCategory)).second;
			ItemWidth = WeaponItemData->ItemWidth;
			ItemHeight = WeaponItemData->ItemHeight;
			ItemMinDamage = WeaponItemData->ItemMinDamage;
			ItemMaxDamage = WeaponItemData->ItemMaxDamage;
		}
		break;
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_WOOD:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER:
		{
			ItemIsQuickSlotUse = false;
			ItemLargeCategory = (int8)DropItemData.LargeItemCategory;
			ItemMediumCategory = (int8)DropItemData.MediumItemCategory;
			ItemSmallCategory = (int16)DropItemData.SmallItemCategory;
			ItemName = (LPWSTR)CA2W(DropItemData.ItemName.c_str());
			ItemCount = DropItemData.ItemCount;
			ItemEquipped = false;
			ItemThumbnailImagePath = (LPWSTR)CA2W(DropItemData.ItemThumbnailImagePath.c_str());

			st_ItemData* ArmorItemData = (*G_Datamanager->_Items.find((int16)DropItemData.SmallItemCategory)).second;
			ItemWidth = ArmorItemData->ItemWidth;
			ItemHeight = ArmorItemData->ItemHeight;
			ItemDefence = ArmorItemData->ItemDefence;
		}
		break;
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_HEAL_SMALL:
			break;
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHOHONE_ATTACK:
			break;
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_LEATHER:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIMEGEL:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_BRONZE_COIN:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIVER_COIN:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_GOLD_COIN:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_STONE:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_YARN:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_CHAR_COAL:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_COPPER_NUGGET:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_COPPER_INGOT:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_IRON_NUGGET:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_IRON_INGOT:
			{
				ItemIsQuickSlotUse = false;
				ItemLargeCategory = (int8)DropItemData.LargeItemCategory;
				ItemMediumCategory = (int8)DropItemData.MediumItemCategory;
				ItemSmallCategory = (int16)DropItemData.SmallItemCategory;
				ItemName = (LPWSTR)CA2W(DropItemData.ItemName.c_str());
				ItemCount = DropItemData.ItemCount;
				ItemEquipped = false;
				ItemThumbnailImagePath = (LPWSTR)CA2W(DropItemData.ItemThumbnailImagePath.c_str());

				st_ItemData* MaterialItemData = (*G_Datamanager->_Items.find((int16)DropItemData.SmallItemCategory)).second;
				ItemWidth = MaterialItemData->ItemWidth;
				ItemHeight = MaterialItemData->ItemHeight;
				ItemMaxCount = MaterialItemData->ItemMaxCount;
			}
			break;		
		}

		// 아이템 생성
		CItem* NewItem = ItemCreate((en_SmallItemCategory)ItemSmallCategory);
		
		NewItem->_ItemInfo.ItemDBId = NewItem->_GameObjectInfo.ObjectId;
		NewItem->_ItemInfo.ItemIsQuickSlotUse = ItemIsQuickSlotUse;
		NewItem->_ItemInfo.Width = ItemWidth;
		NewItem->_ItemInfo.Height = ItemHeight;
		NewItem->_ItemInfo.TileGridPositionX = ItemTilePositionX;
		NewItem->_ItemInfo.TileGridPositionY = ItemTilePositionY;
		NewItem->_ItemInfo.ItemLargeCategory = (en_LargeItemCategory)ItemLargeCategory;
		NewItem->_ItemInfo.ItemMediumCategory = (en_MediumItemCategory)ItemMediumCategory;
		NewItem->_ItemInfo.ItemSmallCategory = (en_SmallItemCategory)ItemSmallCategory;
		NewItem->_ItemInfo.ItemName = ItemName;
		NewItem->_ItemInfo.ItemMaxCount = ItemMaxCount;
		NewItem->_ItemInfo.ItemCount = ItemCount;
		NewItem->_ItemInfo.ItemThumbnailImagePath = ItemThumbnailImagePath;
		NewItem->_ItemInfo.ItemIsEquipped = ItemEquipped;

		NewItem->_GameObjectInfo.ObjectType = DropItemData.ItemObjectType;		
		NewItem->_GameObjectInfo.OwnerObjectId = KillerId;
		NewItem->_GameObjectInfo.OwnerObjectType = (en_GameObjectType)KillerObjectType;
		NewItem->_SpawnPosition = SpawnPosition;

		// 아이템 월드에 스폰
		G_ObjectManager->ObjectEnterGame(NewItem, 1);	
	}
}

void CObjectManager::ObjectItemDropToSpawn(en_SmallItemCategory DropItemType, int32 DropItemCount, st_Vector2Int SpawnPosition)
{
	CItem* NewItem = ItemCreate(DropItemType);
	if (NewItem != nullptr)
	{
		st_ItemData* ItemData = G_Datamanager->FindItemData(DropItemType);
		if (ItemData != nullptr)
		{
			NewItem->_ItemInfo.ItemDBId = NewItem->_GameObjectInfo.ObjectId;
			NewItem->_ItemInfo.ItemIsQuickSlotUse = false;
			NewItem->_ItemInfo.Width = ItemData->ItemWidth;
			NewItem->_ItemInfo.Height = ItemData->ItemHeight;
			NewItem->_ItemInfo.TileGridPositionX = 0;
			NewItem->_ItemInfo.TileGridPositionY = 0;
			NewItem->_ItemInfo.ItemLargeCategory = ItemData->LargeItemCategory;
			NewItem->_ItemInfo.ItemMediumCategory = ItemData->MediumItemCategory;
			NewItem->_ItemInfo.ItemSmallCategory = ItemData->SmallItemCategory;
			NewItem->_ItemInfo.ItemName = (LPWSTR)CA2W(ItemData->ItemName.c_str());
			NewItem->_ItemInfo.ItemCount = DropItemCount;
			NewItem->_ItemInfo.ItemMaxCount = ItemData->ItemMaxCount;
			NewItem->_ItemInfo.ItemThumbnailImagePath = (LPWSTR)CA2W(ItemData->ItemThumbnailImagePath.c_str());
			NewItem->_ItemInfo.ItemIsEquipped = false;

			NewItem->_GameObjectInfo.ObjectType = ItemData->ItemObjectType;
			NewItem->_GameObjectInfo.OwnerObjectId = 0;
			NewItem->_GameObjectInfo.OwnerObjectType = en_GameObjectType::NORMAL;
			NewItem->_SpawnPosition = SpawnPosition;

			G_ObjectManager->ObjectEnterGame(NewItem, 1);
		}
	}	
}

void CObjectManager::ObjectSpawn(en_GameObjectType ObjectType, st_Vector2Int SpawnPosition)
{
	CGameObject* SpawnGameObject = nullptr;

	switch (ObjectType)
	{
	case en_GameObjectType::OBJECT_SLIME:
		SpawnGameObject = ObjectCreate(en_GameObjectType::OBJECT_SLIME);
		break;
	case en_GameObjectType::OBJECT_BEAR:
		SpawnGameObject = ObjectCreate(en_GameObjectType::OBJECT_BEAR);
		break;
	case en_GameObjectType::OBJECT_STONE:
		SpawnGameObject = ObjectCreate(en_GameObjectType::OBJECT_STONE);
		break;
	case en_GameObjectType::OBJECT_TREE:
		SpawnGameObject = ObjectCreate(en_GameObjectType::OBJECT_TREE);
		break;
	default:
		break;
	}

	if (SpawnGameObject != nullptr)
	{
		SpawnGameObject->_GameObjectInfo.ObjectId = InterlockedIncrement64(&_GameServerObjectId);
		SpawnGameObject->_SpawnPosition = SpawnPosition;
		ObjectEnterGame(SpawnGameObject, 1);
	}
}

st_GameObjectJob* CObjectManager::GameObjectJobCreate()
{
	return _GameObjectJobMemoryPool->Alloc();
}

void CObjectManager::GameObjectJobReturn(st_GameObjectJob* GameObjectJob)
{
	_GameObjectJobMemoryPool->Free(GameObjectJob);
}
