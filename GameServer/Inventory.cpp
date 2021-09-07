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
		InitItemInfo->ItemDBId = 0;
		InitItemInfo->ItemType = en_ItemType::ITEM_TYPE_NONE;
		InitItemInfo->ItemConsumableType = en_ConsumableType::NONE;
		InitItemInfo->ItemName = L"";
		InitItemInfo->ItemCount = 0;
		InitItemInfo->ThumbnailImagePath = L"";
		InitItemInfo->IsEquipped = false;
		InitItemInfo->SlotIndex = SlotIndex;

		InitItemInfo->IsEquipped = false;
		_Items.insert(pair<byte, st_ItemInfo*>(SlotIndex, InitItemInfo));
	}

	_BronzeCoinCount = 0;
	_SliverCoinCount = 0;
	_GoldCoinCount = 0;
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

	(*FindSlotIterator).second->ItemDBId = Item->_ItemInfo.ItemDBId;
	(*FindSlotIterator).second->ItemType = Item->_ItemInfo.ItemType;
	(*FindSlotIterator).second->ItemName = Item->_ItemInfo.ItemName;
	(*FindSlotIterator).second->ItemCount = Item->_ItemInfo.ItemCount;
	(*FindSlotIterator).second->ThumbnailImagePath = Item->_ItemInfo.ThumbnailImagePath;
	(*FindSlotIterator).second->IsEquipped = Item->_ItemInfo.IsEquipped;
	(*FindSlotIterator).second->SlotIndex = Item->_ItemInfo.SlotIndex;
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

bool CInventory::IsExistItem(en_ItemType ItemType, int16* Count, int8* SlotIndex)
{
	for (auto ItemIteraotr : _Items)
	{
		st_ItemInfo* Item = ItemIteraotr.second;

		if (Item != nullptr && Item->ItemType == ItemType)
		{
			Item->ItemCount += 1;
			*Count = Item->ItemCount;
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
		(*FindSwapAItem).second->ItemDBId = SwapAItemInfo.ItemDBId;
		(*FindSwapAItem).second->ItemType = SwapAItemInfo.ItemType;
		(*FindSwapAItem).second->ItemConsumableType = SwapAItemInfo.ItemConsumableType;
		(*FindSwapAItem).second->ItemName = SwapAItemInfo.ItemName;
		(*FindSwapAItem).second->ItemCount = SwapAItemInfo.ItemCount;
		(*FindSwapAItem).second->ThumbnailImagePath = SwapAItemInfo.ThumbnailImagePath;
		(*FindSwapAItem).second->IsEquipped = SwapAItemInfo.IsEquipped;
		(*FindSwapAItem).second->SlotIndex = SwapAItemInfo.SlotIndex;		
	}

	auto FindSwapBItem = _Items.find(SwapBItemInfo.SlotIndex);
	if (FindSwapBItem != _Items.end())
	{
		(*FindSwapBItem).second->ItemDBId = SwapBItemInfo.ItemDBId;
		(*FindSwapBItem).second->ItemType = SwapBItemInfo.ItemType;
		(*FindSwapBItem).second->ItemConsumableType = SwapBItemInfo.ItemConsumableType;
		(*FindSwapBItem).second->ItemName = SwapBItemInfo.ItemName;
		(*FindSwapBItem).second->ItemCount = SwapBItemInfo.ItemCount;
		(*FindSwapBItem).second->ThumbnailImagePath = SwapBItemInfo.ThumbnailImagePath;
		(*FindSwapBItem).second->IsEquipped = SwapBItemInfo.IsEquipped;
		(*FindSwapBItem).second->SlotIndex = SwapBItemInfo.SlotIndex;
	}
}