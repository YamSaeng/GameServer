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
	_BronzeCoinCount = 0;
	_SliverCoinCount = 0;
	_GoldCoinCount = 0;

	CInventory* NewInventory = new CInventory();
	
	NewInventory->Init(Width, Height);
	_Inventorys[0] = NewInventory;
}

void CInventoryManager::InsertMoney(int8 SelectInventoryIndex, CItem* InsertMoneyItem)
{
	if (InsertMoneyItem != nullptr)
	{
		int16 SliverCoinCount = 0;
		int64 GoldCoinCount = 0;

		st_Vector2Int GridPosition;

		switch (InsertMoneyItem->_ItemInfo.ItemSmallCategory)
		{
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_BRONZE_COIN:
			_BronzeCoinCount += (int8)(InsertMoneyItem->_ItemInfo.ItemCount);
			SliverCoinCount = (int8)(_BronzeCoinCount / 100);

			if (SliverCoinCount > 0)
			{
				_BronzeCoinCount = (int8)(_BronzeCoinCount - (SliverCoinCount * 100));
			}
			else
			{
				_BronzeCoinCount += (int8)(SliverCoinCount * 100);
			}

			_SliverCoinCount += SliverCoinCount;
			break;
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIVER_COIN:
			_SliverCoinCount += (byte)(InsertMoneyItem->_ItemInfo.ItemCount);
			GoldCoinCount = (byte)(_SliverCoinCount / 100);

			if (GoldCoinCount > 0)
			{
				_SliverCoinCount = (byte)(_SliverCoinCount - (GoldCoinCount * 100));
			}
			else
			{
				_SliverCoinCount += (byte)(GoldCoinCount * 100);
			}

			_GoldCoinCount += GoldCoinCount;
			break;
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_GOLD_COIN:
			_GoldCoinCount += (int8)(InsertMoneyItem->_ItemInfo.ItemCount);
			break;		
		default:
			break;
		}
	}
}

void CInventoryManager::InsertItem(int8 SelectInventoryIndex, CItem* InsertNewItem)
{
	if (InsertNewItem != nullptr)
	{
		st_Vector2Int GridPosition = _Inventorys[SelectInventoryIndex]->FindEmptySpace(InsertNewItem);
		if (GridPosition._X == -1 && GridPosition._Y == -1)
		{
			return;
		}

		_Inventorys[SelectInventoryIndex]->PlaceItem(InsertNewItem, GridPosition._X, GridPosition._Y);
	}	
	else
	{
		CRASH("InsertItem InsertNewITem이 존재하지 않음");
	}
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

CItem* CInventoryManager::FindInventoryItem(int8 SelectInventoryIndex, en_SmallItemCategory FindItemSmallItemCategory)
{
	return _Inventorys[SelectInventoryIndex]->FindInventoryItem(FindItemSmallItemCategory);
}

vector<CItem*> CInventoryManager::FindAllInventoryItem(int8 SelectInventoryIndex, en_SmallItemCategory FindItemSmallItemCategory)
{
	return _Inventorys[SelectInventoryIndex]->FindAllInventoryItem(FindItemSmallItemCategory);
}

