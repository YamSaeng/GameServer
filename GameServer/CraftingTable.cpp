#include "pch.h"
#include "CraftingTable.h"
#include "Item.h"
#include "ObjectManager.h"
#include "RectCollision.h"

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

st_CraftingTableRecipe CCraftingTable::GetCraftingTableRecipe()
{
	return _CraftingTableRecipe;
}

void CCraftingTable::InputMaterialItem(CItem* MaterialItem, int16 MaterialItemCount)
{
	// 우선 용광로 재료 템들 중에 넣을 아이템이 존재하는지 확인
	auto FindMaterialIter = _MaterialItems.find(MaterialItem->_ItemInfo.ItemSmallCategory);
	// 발견하지 못햇을 경우
	if (FindMaterialIter == _MaterialItems.end())
	{
		// 아이템 생성
		CItem* NewMaterialItem = G_ObjectManager->ItemCreate(MaterialItem->_ItemInfo.ItemSmallCategory);
		if (NewMaterialItem != nullptr)
		{				
			NewMaterialItem->_ItemInfo.ItemTileGridPositionX = MaterialItem->_ItemInfo.ItemTileGridPositionX;
			NewMaterialItem->_ItemInfo.ItemTileGridPositionY = MaterialItem->_ItemInfo.ItemTileGridPositionY;									
			NewMaterialItem->_ItemInfo.ItemCount = MaterialItemCount;
			NewMaterialItem->_ItemInfo.ItemIsEquipped = MaterialItem->_ItemInfo.ItemIsEquipped;

			_MaterialItems.insert(pair<en_SmallItemCategory, CItem*>(NewMaterialItem->_ItemInfo.ItemSmallCategory, NewMaterialItem));
		}
	}
	else
	{
		// 발견 했을 경우 개수만 늘려줌
		(*FindMaterialIter).second->_ItemInfo.ItemCount += MaterialItemCount;
	}
}

bool CCraftingTable::FindMaterialItem(en_SmallItemCategory FindSmallItemCategory, int16 ItemCount)
{
	auto FindMaterialIter = _MaterialItems.find(FindSmallItemCategory);
	if (FindMaterialIter == _MaterialItems.end())
	{
		return false;
	}

	if ((*FindMaterialIter).second->_ItemInfo.ItemCount >= ItemCount)
	{
		return true;
	}
	else
	{
		return false;
	}
}
