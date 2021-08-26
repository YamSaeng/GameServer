#include "pch.h"
#include "Inventory.h"

CInventory::CInventory()
{
	_Items = (CItem**)malloc(sizeof(CItem*) * 16);		
	memset(_Items, 0, sizeof(CItem*) * 16);

	for (int32 SlotIndex = 16 - 1; SlotIndex >= 0; SlotIndex--)
	{
		_ItemsSlotIndex.Push(SlotIndex);
	}	

	_Money = 0;
}

CInventory::~CInventory()
{

}

void CInventory::Add(CItem* Item)
{
	if (Item == nullptr)
	{
		CRASH("Inventory Item is null");
		return;
	}

	_Items[Item->_ItemInfo.SlotNumber] = Item;
}

CItem* CInventory::Get(int32 _ItemDBId)
{
	CItem* Item = nullptr;

	for (int32 i = 0; i < 16; i++)
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
	for (int32 i = 0; i < 16; i++)
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
			case ITEM_TYPE_BRONZE_COIN:
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
