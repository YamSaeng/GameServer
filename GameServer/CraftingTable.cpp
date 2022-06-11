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

void CCraftingTable::CraftingStart()
{
	_CraftingTick = GetTickCount64() + 5000;

	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::CRAFTING;
}