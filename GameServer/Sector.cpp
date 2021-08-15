#include "pch.h"
#include "Sector.h"
#include "Player.h"
#include "Monster.h"

CSector::CSector(int32 SectorY, int32 SectorX)
{
	_SectorY = SectorY;
	_SectorX = SectorX;
}

void CSector::Insert(CGameObject* InsertGameObject)
{
	switch (InsertGameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::PLAYER:
		_Players.insert((CPlayer*)InsertGameObject);
		break;
	case en_GameObjectType::MONSTER:
		_Monsters.insert((CMonster*)InsertGameObject);
		break;
	default:
		break;
	}
}

void CSector::Remove(CGameObject* RemoveGameObject)
{
	switch (RemoveGameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::PLAYER:
		_Players.erase((CPlayer*)RemoveGameObject);
		break;
	case en_GameObjectType::MONSTER:
		_Monsters.erase((CMonster*)RemoveGameObject);
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
