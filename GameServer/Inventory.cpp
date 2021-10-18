#include "pch.h"
#include "Inventory.h"
#include "ObjectManager.h"

CInventory::CInventory()
{

}

CInventory::~CInventory()
{
	for (auto ItemIteraotr : _Items)
	{
		st_ItemInfo* Item = ItemIteraotr.second;
		delete Item;		
	}
}

void CInventory::Init()
{
	// INVENTORY_SIZE 만큼 할당
	for (int8 SlotIndex = 0; SlotIndex < (int8)en_Inventory::INVENTORY_SIZE; SlotIndex++)
	{
		// 빈껍데기 정보 생성
		st_ItemInfo* InitItemInfo = new st_ItemInfo();		
		InitItemInfo->SlotIndex = SlotIndex;

		InitItemInfo->IsEquipped = false;
		_Items.insert(pair<byte, st_ItemInfo*>(SlotIndex, InitItemInfo));
	}

	_BronzeCoinCount = 0;
	_SliverCoinCount = 0;
	_GoldCoinCount = 0;
}

void CInventory::AddItem(st_ItemInfo& ItemInfo)
{
	if (ItemInfo.ItemType == en_ItemType::ITEM_TYPE_NONE)
	{
		CRASH("Inventory Item is null");
		return;
	}

	auto FindSlotIterator = _Items.find(ItemInfo.SlotIndex);
	if (FindSlotIterator == _Items.end())
	{
		return;
	}

	*(*FindSlotIterator).second = ItemInfo;	
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

st_ItemInfo* CInventory::Get(int8 SlotIndex)
{
	auto FindItemIterator = _Items.find(SlotIndex);
	if (FindItemIterator == _Items.end())
	{
		return nullptr;
	}

	return (*FindItemIterator).second;
}

bool CInventory::IsExistItem(en_ItemType ItemType, int16& ItemEach, int8* SlotIndex)
{
	for (auto ItemIteraotr : _Items)
	{
		st_ItemInfo* Item = ItemIteraotr.second;

		if (Item != nullptr && Item->ItemType == ItemType)
		{
			Item->ItemCount += ItemEach;
			*SlotIndex = ItemIteraotr.first;
			return true;
		}
	}

	return false;
}

vector<st_ItemInfo*> CInventory::Find(en_ItemType ItemType)
{
	vector<st_ItemInfo*> FindItem;

	for (auto ItemIterator : _Items)
	{
		st_ItemInfo* Item = ItemIterator.second;

		if (Item->ItemType == ItemType)
		{
			FindItem.push_back(Item);
		}
	}

	return FindItem;
}

bool CInventory::GetEmptySlot(int8* SlotIndex)
{
	for (auto ItemIteraotr : _Items)
	{
		st_ItemInfo* Item = ItemIteraotr.second;

		if (Item->ItemType == en_ItemType::ITEM_TYPE_NONE)
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
		*(*FindSwapAItem).second = SwapAItemInfo;		
	}

	auto FindSwapBItem = _Items.find(SwapBItemInfo.SlotIndex);
	if (FindSwapBItem != _Items.end())
	{
		*(*FindSwapBItem).second = SwapBItemInfo;		
	}
}