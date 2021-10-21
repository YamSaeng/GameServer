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
		CItem* Item = ItemIteraotr.second;
		delete Item;		
	}
}

void CInventory::Init()
{
	_BronzeCoinCount = 0;
	_SliverCoinCount = 0;
	_GoldCoinCount = 0;
}

void CInventory::AddItem(CItem* Item)
{
	// ������ ���� �߰��� ���, 100���� �Ѱ� �Ǹ�
	// ���� ������ 100���� 1���� ����ϰ� ������ ���ϰ� �߰��Ѵ�.
	byte SliverCoinCount;
	int64 GoldCoinCount;

	switch (Item->_ItemInfo.ItemSmallCategory)
	{
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD:		
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_WOOD:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER:		
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_HEAL_SMALL:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_CHOHONE:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_LEATHER:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIMEGEL:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_STONE:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_YARN:
		{
			auto FindSlotIterator = _Items.find(Item->_ItemInfo.SlotIndex);
			if (FindSlotIterator == _Items.end())
			{
				return;
			}

			(*FindSlotIterator).second = Item;
		}
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_BRONZE_COIN:
		{			
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
		}	
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIVER_COIN:
		{
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
		}			
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_GOLD_COIN:
		{
			CMaterial* Coin = (CMaterial*)Item;

			_GoldCoinCount += Item->_ItemInfo.ItemCount;
		}		
		break;							
	}
}

CItem* CInventory::Get(int8 SlotIndex)
{
	auto FindItemIterator = _Items.find(SlotIndex);
	if (FindItemIterator == _Items.end())
	{
		return nullptr;
	}

	return (*FindItemIterator).second;
}

bool CInventory::IsExistItem(en_SmallItemCategory ItemType, int16& ItemEach, int8* SlotIndex)
{
	switch (ItemType)
	{	
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_WOOD:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER:
		{
			// ����� ���� ��� �ߺ��� �ȵǹǷ� ��� �ִ� ���� �ε����� ��ȯ�Ѵ�.
			GetEmptySlot(SlotIndex);
		}	
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_HEAL_SMALL:		
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_CHOHONE:		
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_LEATHER:		
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIMEGEL:			
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_STONE:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_YARN:
		{
			// ����� ��� �ߺ��� ���� �ϹǷ� 
			for (auto ItemIterator : _Items)
			{
				CItem* Item = ItemIterator.second;				

				if (Item != nullptr && Item->_ItemInfo.ItemSmallCategory == ItemType)
				{
					CMaterial* Material = (CMaterial*)Item;

					// ����� ������ ����
					Item->_ItemInfo.ItemCount += ItemEach;
					// �۾��� ������ �ε����� ��ȯ
					*SlotIndex = ItemIterator.first;
					return true;
				}
			}
		}
		break;
	}

	return false;
}

vector<CItem*> CInventory::Find(en_SmallItemCategory ItemType)
{
	// �κ��丮���� ItemType�� ���� ��� �������� ã�Ƽ� ��ȯ�Ѵ�.
	vector<CItem*> FindItem;

	for (auto ItemIterator : _Items)
	{
		CItem* Item = (CItem*)ItemIterator.second;

		if (Item != nullptr && Item->_ItemInfo.ItemSmallCategory == ItemType)
		{
			FindItem.push_back(Item);
		}
	}

	return FindItem;
}

bool CInventory::GetEmptySlot(int8* SlotIndex)
{
	// ����� ���� ��� �ߺ��� �ȵǹǷ� ��� �ִ� ���� �ε����� ��ȯ�Ѵ�.
	for (auto ItemIterator : _Items)
	{
		CItem* Item = ItemIterator.second;

		if (Item->_ItemInfo.ItemSmallCategory == en_SmallItemCategory::ITEM_SMALL_CATEGORY_NONE)
		{
			*SlotIndex = ItemIterator.first;
			return true;
		}
	}

	return false;
}

void CInventory::SwapItem(CItem* SwapAItem, CItem* SwapBItem)
{
	auto FindSwapAItem = _Items.find(SwapAItem->_ItemInfo.SlotIndex);
	if (FindSwapAItem != _Items.end())
	{
		(*FindSwapAItem).second = SwapAItem;
	}

	auto FindSwapBItem = _Items.find(SwapBItem->_ItemInfo.SlotIndex);
	if (FindSwapBItem != _Items.end())
	{
		(*FindSwapBItem).second = SwapBItem;
	}
}