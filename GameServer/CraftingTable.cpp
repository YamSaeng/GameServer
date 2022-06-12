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

void CCraftingTable::CraftingStart(int64 CraftingTime)
{
	_CraftingTick = CraftingTime + GetTickCount64();

	_CraftingRemainTime = _CraftingTick - GetTickCount64();

	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::CRAFTING;	
}

void CCraftingTable::CraftingStop()
{
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
}

map<en_SmallItemCategory, CItem*> CCraftingTable::GetMaterialItems()
{
	return _MaterialItems;
}

map<en_SmallItemCategory, CItem*> CCraftingTable::GetCompleteItems()
{
	return _CompleteItems;
}
