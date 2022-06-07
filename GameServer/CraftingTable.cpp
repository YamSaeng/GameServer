#include "pch.h"
#include "CraftingTable.h"

CCraftingTable::CCraftingTable()
{
	_SelectedCraftingTable = false;
}

void CCraftingTable::Start()
{
	_SpawnPosition = _GameObjectInfo.ObjectPositionInfo.CollisionPosition;
}
