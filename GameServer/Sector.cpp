#include "pch.h"
#include "Sector.h"
#include "Player.h"
#include "Monster.h"
#include "Item.h"

CSector::CSector(int32 SectorY, int32 SectorX)
{
	_SectorY = SectorY;
	_SectorX = SectorX;
}

void CSector::Insert(CGameObject* InsertGameObject)
{
	switch (InsertGameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_MELEE_PLAYER:
	case en_GameObjectType::OBJECT_MAGIC_PLAYER:
		_Players.insert((CPlayer*)InsertGameObject);
		break;
	case en_GameObjectType::OBJECT_SLIME:
	case en_GameObjectType::OBJECT_BEAR:
		_Monsters.insert((CMonster*)InsertGameObject);
		break;
	case en_GameObjectType::ITEM_SLIME_GEL:
	case en_GameObjectType::ITEM_BRONZE_COIN:
	case en_GameObjectType::ITEM_LEATHER:
	case en_GameObjectType::ITEM_SKILL_BOOK:
	case en_GameObjectType::ITEM_WOOD_LOG:
	case en_GameObjectType::ITEM_STONE:
		_Items.insert((CItem*)InsertGameObject);
		break;
	case en_GameObjectType::OBJECT_STONE:
	case en_GameObjectType::OBJECT_TREE:
		_Environment.insert((CEnvironment*)InsertGameObject);
		break;
	default:
		break;
	}
}

void CSector::Remove(CGameObject* RemoveGameObject)
{
	switch (RemoveGameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_MELEE_PLAYER:
	case en_GameObjectType::OBJECT_MAGIC_PLAYER:
		_Players.erase((CPlayer*)RemoveGameObject);
		break;
	case en_GameObjectType::OBJECT_SLIME:
	case en_GameObjectType::OBJECT_BEAR:
		_Monsters.erase((CMonster*)RemoveGameObject);
		break;
	case en_GameObjectType::ITEM_SLIME_GEL:
	case en_GameObjectType::ITEM_BRONZE_COIN:
	case en_GameObjectType::ITEM_LEATHER:
	case en_GameObjectType::ITEM_SKILL_BOOK:
	case en_GameObjectType::ITEM_WOOD_LOG:
	case en_GameObjectType::ITEM_STONE:
		_Items.erase((CItem*)RemoveGameObject);
		break;
	case en_GameObjectType::OBJECT_STONE:
	case en_GameObjectType::OBJECT_TREE:
		_Environment.erase((CEnvironment*)RemoveGameObject);
		break;
	}
}

set<CPlayer*> CSector::GetPlayers()
{
	return _Players;
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
