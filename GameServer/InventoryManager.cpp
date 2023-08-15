#include "pch.h"
#include "InventoryManager.h"
#include "ObjectManager.h"
#include "DataManager.h"
#include <atlbase.h>

CInventoryManager::CInventoryManager()
{
}

CInventoryManager::~CInventoryManager()
{
	for (int i = 0; i < _InventoryCount; i++)
	{
		if (_Inventorys[i] != nullptr)
		{
			delete _Inventorys[i];
			_Inventorys[i] = nullptr;
		}
	}	
}

int8 CInventoryManager::GetInventoryCount()
{
	return _InventoryCount;
}

vector<CInventory*> CInventoryManager::GetInventoryManager()
{
	return _Inventorys;
}

void CInventoryManager::InventoryCreate(int8 InventoryCount, int8 Width, int8 Height)
{
	_InventoryCount = InventoryCount;
	
	_Coin = 0;
	
	for (int i = 0; i < InventoryCount; i++)
	{
		_Inventorys.push_back(new CInventory);		
		_Inventorys[i]->Init(Width, Height);
	}	
}

void CInventoryManager::InsertMoney(int8 SelectInventoryIndex, int64 Coin)
{
	_Coin += Coin;
}

void CInventoryManager::InsertItem(int8 SelectInventoryIndex, CItem* InsertNewItem)
{
	if (InsertNewItem != nullptr)
	{
		Vector2Int GridPosition = _Inventorys[SelectInventoryIndex]->FindEmptySpace(InsertNewItem);
		if (GridPosition.X == -1 && GridPosition.Y == -1)
		{
			return;
		}

		_Inventorys[SelectInventoryIndex]->PlaceItem(InsertNewItem, GridPosition.X, GridPosition.Y);
	}	
	else
	{
		CRASH("InsertItem InsertNewITem이 존재하지 않음");
	}
}

CItem* CInventoryManager::InsertItem(int8 SelectInventoryIndex, en_SmallItemCategory InsertItemCategory, int16 InsertItemCount, bool* IsExistItem)
{
	CItem* ReturnItem = nullptr;

	// 넣고자 하는 아이템이 인벤토리에 있는지 우선 확인
	CItem* FindItem = FindInventoryItem(0, InsertItemCategory);
	if (FindItem != nullptr)
	{
		*IsExistItem = true;
		
		FindItem->_ItemInfo.ItemCount += InsertItemCount;

		ReturnItem = FindItem;
	}
	else
	{
		*IsExistItem = false;

		CItem* NewItem = G_ObjectManager->ItemCreate(InsertItemCategory);		
		NewItem->_ItemInfo.ItemCount = InsertItemCount;
		
		InsertItem(SelectInventoryIndex, NewItem);

		ReturnItem = NewItem;
	}

	return ReturnItem;
}

void CInventoryManager::DBItemInsertItem(int8 SelectInventoryIndex, CItem* NewItem)
{
	if (_Inventorys[SelectInventoryIndex]->FindItemSpaceEmpty(NewItem))
	{
		_Inventorys[SelectInventoryIndex]->PlaceItem(NewItem, NewItem->_ItemInfo.ItemTileGridPositionX, NewItem->_ItemInfo.ItemTileGridPositionY);
	}	
}

void CInventoryManager::DBMoneyInsert(int16 Coin)
{
	_Coin = Coin;	
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
		_Inventorys[SelectInventoryIndex]->PlaceItem(_SelectItem, _SelectItem->_ItemInfo.ItemTileGridPositionX, _SelectItem->_ItemInfo.ItemTileGridPositionY);		
	}

	_SelectItem = BItem;

	return PlaceItem;
}

CItem* CInventoryManager::GetItem(int8 SelectInventoryIndex, int8 TilePositionX, int8 TilePositionY)
{
	return _Inventorys[SelectInventoryIndex]->GetItem(TilePositionX, TilePositionY);
}

Vector2Int CInventoryManager::FindEmptySpace(int8 SelectInventoryIndex, CItem* ItemInfo)
{
	return _Inventorys[SelectInventoryIndex]->FindEmptySpace(ItemInfo);
}

CItem* CInventoryManager::FindInventoryItem(int8 SelectInventoryIndex, en_SmallItemCategory FindItemSmallItemCategory)
{
	// 가방 안에 찾고자 하는 아이템이 있는지 확인
	CItem* FindItem = _Inventorys[SelectInventoryIndex]->FindInventoryItem(FindItemSmallItemCategory);
	if (FindItem == nullptr)
	{
		// 없을 경우 선택하고 잇는 아이템을 추가로 검사
		if (_SelectItem != nullptr && _SelectItem->_ItemInfo.ItemSmallCategory == FindItemSmallItemCategory)
		{
			return _SelectItem;
		}		
	}	

	return FindItem;
}

void CInventoryManager::InitItem(int8 SelectInventoryIndex, int8 TilePositionX, int8 TilePositionY)
{
	_Inventorys[SelectInventoryIndex]->InitItem(TilePositionX, TilePositionY);
}

vector<CItem*> CInventoryManager::FindAllInventoryItem(int8 SelectInventoryIndex, en_SmallItemCategory FindItemSmallItemCategory)
{
	return _Inventorys[SelectInventoryIndex]->FindAllInventoryItem(FindItemSmallItemCategory);
}

int16 CInventoryManager::GetCoin()
{
	return _Coin;
}