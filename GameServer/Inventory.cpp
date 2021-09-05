#include "pch.h"
#include "Inventory.h"

CInventory::CInventory()
{
	// INVENTORY_SIZE 만큼 할당
	for (int8 SlotIndex = 0; SlotIndex < INVENTORY_SIZE; SlotIndex++)
	{
		_Items.insert(pair<byte, CItem*>(SlotIndex,nullptr));
	}	
	
	_BronzeCoinCount = 0;
	_SliverCoinCount = 0;
	_GoldCoinCount = 0;
}

CInventory::~CInventory()
{

}

void CInventory::AddItem(int8 SlotIndex, CItem* Item)
{
	if (Item == nullptr)
	{
		CRASH("Inventory Item is null");
		return;
	}

	auto FindSlotIterator = _Items.find(SlotIndex);
	if (FindSlotIterator == _Items.end())
	{
		return;
	}

	(*FindSlotIterator).second = Item;	
}

void CInventory::AddCoin(CItem* Item)
{
	// 동전과 은전 추가할 경우, 100개가 넘게 되면
	// 다음 동전을 100개당 1개씩 비례하게 개수를 구하고 추가한다.
	byte SliverCoinCount;
	int64 GoldCoinCount;
	switch (Item->_ItemInfo.ItemType)
	{
	case en_ItemType::ITEM_TYPE_BRONZE_COIN:
		_BronzeCoinCount += (byte)(Item->_ItemInfo.ItemCount);
		SliverCoinCount = (byte)(_BronzeCoinCount / 100);

		if (SliverCoinCount > 0)
		{
			_BronzeCoinCount = (byte)(_BronzeCoinCount - (SliverCoinCount * 100));
		}
		else
		{
			_BronzeCoinCount += (byte)(SliverCoinCount * 100);
		}

		_SliverCoinCount += SliverCoinCount;
		break;
	case en_ItemType::ITEM_TYPE_SLIVER_COIN:
		_SliverCoinCount += (byte)(Item->_ItemInfo.ItemCount);
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
	case en_ItemType::ITEM_TYPE_GOLD_COIN:
		_GoldCoinCount += Item->_ItemInfo.ItemCount;
		break;
	}
}

CItem* CInventory::Get(int8 _SlotIndex)
{
	auto FindItemIterator = _Items.find(_SlotIndex);
	if (FindItemIterator == _Items.end())
	{
		return nullptr;
	}

	return (*FindItemIterator).second;
}

bool CInventory::IsExistItem(en_ItemType ItemType, int16* Count, int8* SlotIndex)
{
	for (auto ItemIteraotr : _Items)
	{
		CItem* Item = ItemIteraotr.second;

		if (Item != nullptr && Item->_ItemInfo.ItemType == ItemType)
		{
			Item->_ItemInfo.ItemCount += 1;
			*Count = Item->_ItemInfo.ItemCount;
			*SlotIndex = ItemIteraotr.first;
			return true;
		}
	}

	return false;
}

bool CInventory::GetEmptySlot(int8* SlotIndex)
{
	for (auto ItemIteraotr : _Items)
	{
		CItem* Item = ItemIteraotr.second;

		if (Item == nullptr)
		{
			*SlotIndex = ItemIteraotr.first;
			return true;
		}		
	}		

	return false;
}

void CInventory::SwapItem(st_ItemInfo& SwapAItemInfo, st_ItemInfo& SwapBItemInfo)
{
	auto FindSwapAItem = _Items.find(SwapAItemInfo.SlotIndex);
	if (FindSwapAItem != _Items.end())
	{		
		(*FindSwapAItem).second->_ItemInfo = SwapAItemInfo;
	}

	auto FindSwapBItem = _Items.find(SwapBItemInfo.SlotIndex);
	if (FindSwapBItem != _Items.end())
	{
		(*FindSwapBItem).second->_ItemInfo = SwapBItemInfo;
	}
}