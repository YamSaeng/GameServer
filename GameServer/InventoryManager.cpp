#include "pch.h"
#include "InventoryManager.h"

CInventoryManager::CInventoryManager()
{
}

CInventoryManager::~CInventoryManager()
{
	for (int i = 0; i < en_InventoryManager::INVENTORY_COUNT; i++)
	{
		if (_Inventorys[i] != nullptr)
		{
			delete _Inventorys[i];
		}
	}
}

void CInventoryManager::InventoryCreate(int8 Width, int8 Height)
{
	CInventory* NewInventory = new CInventory();
	
	NewInventory->Init(Width, Height);
	_Inventorys[0] = NewInventory;
}

void CInventoryManager::InsertItem(int8 SelectInventoryIndex, CItem* NewItem)
{
	st_Vector2Int GridPosition = _Inventorys[SelectInventoryIndex]->FindEmptySpace(NewItem);
	if (GridPosition._X == -1 && GridPosition._Y == -1)
	{
		return;
	}

	_Inventorys[SelectInventoryIndex]->PlaceItem(NewItem, GridPosition._X, GridPosition._Y);
}

void CInventoryManager::DBItemInsertItem(int8 SelectInventoryIndex, CItem* NewItem)
{
	if (_Inventorys[SelectInventoryIndex]->FindItemSpaceEmpty(NewItem))
	{
		_Inventorys[SelectInventoryIndex]->PlaceItem(NewItem, NewItem->_ItemInfo.TileGridPositionX, NewItem->_ItemInfo.TileGridPositionY);
	}	
}

CItem* CInventoryManager::SelectItem(int8 SelectInventoryIndex, int8 TilePositionX, int8 TilePositionY)
{
	_SelectItem = _Inventorys[SelectInventoryIndex]->SelectItem(TilePositionX, TilePositionY);	

	return _SelectItem;
}

CItem* CInventoryManager::SwapItem(int8 SelectInventoryIndex, int16 PlaceItemTileGridPositionX, int16 PlaceItemTileGridPositionY)
{
	CItem* BItem = nullptr;
	CItem* PlaceItem = _SelectItem;	

	if (_Inventorys[SelectInventoryIndex]->PlaceItem(_SelectItem, PlaceItemTileGridPositionX, PlaceItemTileGridPositionY, &BItem) == false)
	{
		_Inventorys[SelectInventoryIndex]->PlaceItem(_SelectItem, _SelectItem->_ItemInfo.TileGridPositionX, _SelectItem->_ItemInfo.TileGridPositionY);
	}

	_SelectItem = BItem;

	return PlaceItem;
}

CItem* CInventoryManager::GetItem(int8 SelectInventoryIndex, int8 TilePositionX, int8 TilePositionY)
{
	return _Inventorys[SelectInventoryIndex]->GetItem(TilePositionX, TilePositionY);
}

st_Vector2Int CInventoryManager::FindEmptySpace(int8 SelectInventoryIndex, CItem* ItemInfo)
{
	return _Inventorys[SelectInventoryIndex]->FindEmptySpace(ItemInfo);
}

CItem* CInventoryManager::FindInventoryItem(int8 SelectInventoryIndex, CItem* FindItem)
{
	return _Inventorys[SelectInventoryIndex]->FindInventoryItem(FindItem);
}

