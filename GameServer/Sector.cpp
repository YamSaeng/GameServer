#include "pch.h"
#include "Sector.h"
#include "Player.h"
#include "Monster.h"
#include "Item.h"
#include "CraftingTable.h"

CSector::CSector(int32 SectorY, int32 SectorX)
{
	_SectorY = SectorY;
	_SectorX = SectorX;

	InitializeSRWLock(&_SectorLock);
}

void CSector::Insert(CGameObject* InsertGameObject)
{
	AcquireSRWLockExclusive(&_SectorLock);
		
	switch (InsertGameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_PLAYER:	
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
		_Players.insert((CPlayer*)InsertGameObject);
		break;
	case en_GameObjectType::OBJECT_NON_PLAYER_GENERAL_MERCHANT:
		_NonPlayers.insert((CNonPlayer*)InsertGameObject);
		break;
	case en_GameObjectType::OBJECT_GOBLIN:	
		_Monsters.insert((CMonster*)InsertGameObject);
		break;	
	case en_GameObjectType::OBJECT_WALL:
	case en_GameObjectType::OBJECT_STONE:
	case en_GameObjectType::OBJECT_TREE:
		_Environment.insert((CEnvironment*)InsertGameObject);
		break;
	case en_GameObjectType::OBJECT_SKILL_SWORD_BLADE:
	case en_GameObjectType::OBJECT_SKILL_FLAME_BOLT:
	case en_GameObjectType::OBJECT_SKILL_DIVINE_BOLT:
		_SkillObjects.insert(InsertGameObject);
		break;
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL:
		_CraftingTables.insert((CCraftingTable*)InsertGameObject);
		break;
	case en_GameObjectType::OBJECT_ITEM_CROP_SEED:
	case en_GameObjectType::OBJECT_CROP_POTATO:
	case en_GameObjectType::OBJECT_CROP_CORN:
		_Crops.insert((CCrop*)InsertGameObject);
		break;
	case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_DAGGER:
	case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_LONG_SWORD:
	case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_GREAT_SWORD:
	case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_SHIELD:
	case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_BOW:
	case en_GameObjectType::OBJECT_ITEM_TOOL_FARMING_SHOVEL:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_ARMOR:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_HELMET:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_BOOT:
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_HEALTH_RESTORATION_POTION_SMALL:
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_MANA_RESTORATION_POTION_SMALL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_LEATHER:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_COIN:	
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_LOG:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_STONE:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_FLANK:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_YARN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_FABRIC:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_CHAR_COAL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_COPPER_NUGGET:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_COPPER_INGOT:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_IRON_NUGGET:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_IRON_INGOT:
	case en_GameObjectType::OBJECT_ITEM_CROP_SEED_POTATO:
	case en_GameObjectType::OBJECT_ITEM_CROP_SEED_CORN:
	case en_GameObjectType::OBJECT_ITEM_CROP_FRUIT_POTATO:
	case en_GameObjectType::OBJECT_ITEM_CROP_FRUIT_CORN:
		_Items.insert((CItem*)InsertGameObject);
		break;
	default:
		break;
	}

	ReleaseSRWLockExclusive(&_SectorLock);
}

void CSector::Remove(CGameObject* RemoveGameObject)
{
	AcquireSRWLockExclusive(&_SectorLock);

	switch (RemoveGameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_PLAYER:	
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
		_Players.erase((CPlayer*)RemoveGameObject);
		break;
	case en_GameObjectType::OBJECT_NON_PLAYER_GENERAL_MERCHANT:
		_NonPlayers.erase((CNonPlayer*)RemoveGameObject);
		break;
	case en_GameObjectType::OBJECT_GOBLIN:	
		_Monsters.erase((CMonster*)RemoveGameObject);
		break;	
	case en_GameObjectType::OBJECT_WALL:
	case en_GameObjectType::OBJECT_STONE:
	case en_GameObjectType::OBJECT_TREE:
		_Environment.erase((CEnvironment*)RemoveGameObject);
		break;	
	case en_GameObjectType::OBJECT_SKILL_SWORD_BLADE:
	case en_GameObjectType::OBJECT_SKILL_FLAME_BOLT:
	case en_GameObjectType::OBJECT_SKILL_DIVINE_BOLT:
		_SkillObjects.erase(RemoveGameObject);
		break;
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL:
		_CraftingTables.erase((CCraftingTable*)RemoveGameObject);
		break;
	case en_GameObjectType::OBJECT_ITEM_CROP_SEED:
	case en_GameObjectType::OBJECT_CROP_POTATO:
	case en_GameObjectType::OBJECT_CROP_CORN:
		_Crops.erase((CCrop*)RemoveGameObject);
		break;
	case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_DAGGER:
	case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_LONG_SWORD:
	case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_GREAT_SWORD:
	case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_SHIELD:
	case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_BOW:
	case en_GameObjectType::OBJECT_ITEM_TOOL_FARMING_SHOVEL:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_ARMOR:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_HELMET:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_BOOT:
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_HEALTH_RESTORATION_POTION_SMALL:
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_MANA_RESTORATION_POTION_SMALL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_LEATHER:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_COIN:	
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_LOG:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_STONE:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_FLANK:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_YARN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_FABRIC:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_CHAR_COAL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_COPPER_NUGGET:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_COPPER_INGOT:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_IRON_NUGGET:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_IRON_INGOT:
	case en_GameObjectType::OBJECT_ITEM_CROP_SEED_POTATO:
	case en_GameObjectType::OBJECT_ITEM_CROP_SEED_CORN:
	case en_GameObjectType::OBJECT_ITEM_CROP_FRUIT_POTATO:
	case en_GameObjectType::OBJECT_ITEM_CROP_FRUIT_CORN:
		_Items.erase((CItem*)RemoveGameObject);
		break;
	}

	ReleaseSRWLockExclusive(&_SectorLock);
}

set<CPlayer*> CSector::GetPlayers()
{
	return _Players;
}

set<CNonPlayer*> CSector::GetNonPlayers()
{
	return _NonPlayers;
}

set<CMonster*> CSector::GetMonsters()
{
	return _Monsters;
}

set<CItem*> CSector::GetItems()
{
	return _Items;
}

set<CEnvironment*> CSector::GetEnvironment()
{
	return _Environment;
}

set<CGameObject*> CSector::GetSkillObject()
{
	return _SkillObjects;
}

set<CCraftingTable*> CSector::GetCraftingTable()
{
	return _CraftingTables;
}

set<CCrop*> CSector::GetCrop()
{
	return _Crops;
}

void CSector::AcquireSectorLock()
{
	AcquireSRWLockExclusive(&_SectorLock);
}

void CSector::ReleaseSectorLock()
{
	ReleaseSRWLockExclusive(&_SectorLock);
}
