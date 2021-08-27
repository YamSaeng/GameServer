#include "pch.h"
#include "Inventory.h"

CInventory::CInventory()
{
	_Items = (CItem**)malloc(sizeof(CItem*) * INVENTORY_SIZE);		
	memset(_Items, 0, sizeof(CItem*) * INVENTORY_SIZE);

	for (int32 SlotIndex = INVENTORY_SIZE - 1; SlotIndex >= 0; SlotIndex--)
	{
		_ItemsSlotIndex.Push(SlotIndex);
	}	

	_BronzeCoinCount = 0;
	_SliverCoinCount = 0;
	_GoldCoinCount = 0;
}

CInventory::~CInventory()
{

}

void CInventory::AddItem(CItem* Item)
{
	if (Item == nullptr)
	{
		CRASH("Inventory Item is null");
		return;
	}

	_Items[Item->_ItemInfo.SlotNumber] = Item;
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
		_BronzeCoinCount += (byte)(Item->_ItemInfo.Count);
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
		_SliverCoinCount += (byte)(Item->_ItemInfo.Count);
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
		_GoldCoinCount += Item->_ItemInfo.Count;
		break;
	}
}

CItem* CInventory::Get(int32 _ItemDBId)
{
	CItem* Item = nullptr;

	for (int32 i = 0; i < INVENTORY_SIZE; i++)
	{		
		if (_Items[i] != nullptr && _Items[i]->_GameObjectInfo.ObjectId == _ItemDBId)
		{
			Item = _Items[i];
		}
	}	

	return Item;
}

bool CInventory::IsExistItem(en_ItemType ItemType)
{
	for (int32 i = 0; i < INVENTORY_SIZE; i++)
	{
		if(_Items[i] != nullptr && _Items[i]->_ItemInfo.ItemType == ItemType)
		{
			switch (ItemType)
			{			
			case ITEM_TYPE_WEAPON_SWORD:
				break;
			case ITEM_TYPE_ARMOR_ARMOR:
				break;
			case ITEM_TYPE_ARMOR_HELMET:
				break;
			case ITEM_TYPE_CONSUMABLE_POTION:
				break;
			case ITEM_TYPE_LEATHER:
				break;
			case ITEM_TYPE_SLIMEGEL:
				break;			
			default:
				CRASH("이상한 ItemType");
				break;
			}	

			// 최대 갯수 넘어가면 새로 생성해줘야함
			_Items[i]->_ItemInfo.Count += 1;

			return true;
		}
	}

	return false;
}

bool CInventory::GetEmptySlot(int32* SlotIndex)
{
	return _ItemsSlotIndex.Pop(SlotIndex);	
}
