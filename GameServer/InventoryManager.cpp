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

	_BronzeCoinCount = 0;
	_SliverCoinCount = 0;
	_GoldCoinCount = 0;
	
	for (int i = 0; i < InventoryCount; i++)
	{
		_Inventorys.push_back(new CInventory);		
		_Inventorys[i]->Init(Width, Height);
	}	
}

void CInventoryManager::InsertMoney(int8 SelectInventoryIndex, CItem* InsertMoneyItem)
{
	if (InsertMoneyItem != nullptr)
	{
		int16 SliverCoinCount = 0;
		int64 GoldCoinCount = 0;

		Vector2Int GridPosition;

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

void CInventoryManager::DBMoneyInsert(int64 GoldCoin, int16 SliverCoin, int16 BronzeCoin)
{
	_GoldCoinCount = GoldCoin;
	_SliverCoinCount = SliverCoin;
	_BronzeCoinCount = BronzeCoin;
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

int64 CInventoryManager::GetGoldCoin()
{
	return _GoldCoinCount;
}

int16 CInventoryManager::GetSliverCoin()
{
	return _SliverCoinCount;
}

int16 CInventoryManager::GetBronzeCoin()
{
	return _BronzeCoinCount;
}

