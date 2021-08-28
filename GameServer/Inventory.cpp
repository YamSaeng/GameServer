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

	_Items[Item->_ItemInfo.SlotIndex] = Item;
}

void CInventory::AddCoin(CItem* Item)
{
	// ������ ���� �߰��� ���, 100���� �Ѱ� �Ǹ�
	// ���� ������ 100���� 1���� ����ϰ� ������ ���ϰ� �߰��Ѵ�.
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

bool CInventory::IsExistItem(en_ItemType ItemType, int16* Count, int8* SlotIndex)
{
	for (int32 i = 0; i < INVENTORY_SIZE; i++)
	{
		if(_Items[i] != nullptr && _Items[i]->_ItemInfo.ItemType == ItemType)
		{			
			// �ִ� ���� �Ѿ�� ���� �����������
			_Items[i]->_ItemInfo.ItemCount += 1;
			*Count = _Items[i]->_ItemInfo.ItemCount;
			*SlotIndex = i;

			return true;
		}
	}

	return false;
}

bool CInventory::GetEmptySlot(int8* SlotIndex)
{
	return _ItemsSlotIndex.Pop(SlotIndex);	
}
