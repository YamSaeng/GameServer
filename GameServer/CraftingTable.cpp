#include "pch.h"
#include "CraftingTable.h"

CCraftingTable::CCraftingTable()
{
}

void CCraftingTable::Start()
{
	_SpawnPosition = _GameObjectInfo.ObjectPositionInfo.CollisionPosition;
}
