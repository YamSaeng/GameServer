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

	for (int SlotIndex = 0; SlotIndex < (int8)en_Inventory::INVENTORY_SIZE; SlotIndex++)
	{
		CItem* Item = (CItem*)G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_ITEM);
		Item->_ItemInfo.ItemSlotIndex = SlotIndex;

		_Items.insert(pair<int8, CItem*>(Item->_ItemInfo.ItemSlotIndex,Item));
	}
}

void CInventory::AddItem(CItem* Item)
{
	// 동전과 은전 추가할 경우, 100개가 넘게 되면
	// 다음 동전을 100개당 1개씩 비례하게 개수를 구하고 추가한다.
	byte SliverCoinCount;
	int64 GoldCoinCount;

	switch (Item->_ItemInfo.ItemSmallCategory)
	{
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD:		
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_WOOD:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER:		
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_HEAL_SMALL:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHOHONE_ATTACK:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_LEATHER:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIMEGEL:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_STONE:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_YARN:
		{
			auto FindSlotIterator = _Items.find(Item->_ItemInfo.ItemSlotIndex);
			if (FindSlotIterator == _Items.end())
			{
				return;
			}

			G_ObjectManager->ObjectReturn((*FindSlotIterator).second->_GameObjectInfo.ObjectType, (*FindSlotIterator).second);
			
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
			// 무기와 방어구의 경우 중복이 안되므로 비어 있는 슬롯 인덱스를 반환한다.
			GetEmptySlot(SlotIndex);
		}	
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_HEAL_SMALL:		
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHOHONE_ATTACK:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_LEATHER:		
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIMEGEL:			
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_STONE:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_YARN:
		{
			// 재료의 경우 중복이 가능 하므로 
			for (auto ItemIterator : _Items)
			{
				CItem* Item = ItemIterator.second;				

				if (Item != nullptr && Item->_ItemInfo.ItemSmallCategory == ItemType)
				{
					CMaterial* Material = (CMaterial*)Item;

					// 재료의 개수를 증가
					Item->_ItemInfo.ItemCount += ItemEach;
					// 작업한 슬롯의 인덱스를 반환
					*SlotIndex = ItemIterator.first;
					return true;
				}
			}
		}
		break;
	}

	return false;
}

vector<CItem*> CInventory::Find(en_LargeItemCategory ItemLargeType)
{
	vector<CItem*> FindItem;

	for (auto ItemIterator : _Items)
	{
		CItem* Item = (CItem*)ItemIterator.second;

		if (Item != nullptr && Item->_ItemInfo.ItemLargeCategory == ItemLargeType)
		{
			FindItem.push_back(Item);
		}
	}

	return FindItem;
}

vector<CItem*> CInventory::Find(en_MediumItemCategory ItemMediumType)
{
	vector<CItem*> FindItem;

	for (auto ItemIterator : _Items)
	{
		CItem* Item = (CItem*)ItemIterator.second;

		if (Item != nullptr && Item->_ItemInfo.ItemMediumCategory == ItemMediumType)
		{
			FindItem.push_back(Item);
		}
	}

	return FindItem;
}

vector<CItem*> CInventory::Find(en_SmallItemCategory ItemSmallType)
{
	// 인벤토리에서 ItemType과 같은 모든 아이템을 찾아서 반환한다.
	vector<CItem*> FindItem;

	for (auto ItemIterator : _Items)
	{
		CItem* Item = (CItem*)ItemIterator.second;

		if (Item != nullptr && Item->_ItemInfo.ItemSmallCategory == ItemSmallType)
		{
			FindItem.push_back(Item);
		}
	}

	return FindItem;
}

bool CInventory::GetEmptySlot(int8* SlotIndex)
{
	// 무기와 방어구의 경우 중복이 안되므로 비어 있는 슬롯 인덱스를 반환한다.
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

void CInventory::SwapItem(int8 SwapAIndex, int8 SwapBIndex)
{
	CItem* AItem = Get(SwapAIndex);
	CItem* BItem = Get(SwapBIndex);

	AItem->_ItemInfo.ItemSlotIndex = SwapBIndex;
	BItem->_ItemInfo.ItemSlotIndex = SwapAIndex;

	auto FindSwapAItem = _Items.find(SwapAIndex);
	if (FindSwapAItem != _Items.end())
	{
		(*FindSwapAItem).second = BItem;
	}

	auto FindSwapBItem = _Items.find(SwapBIndex);
	if (FindSwapBItem != _Items.end())
	{
		(*FindSwapBItem).second = AItem;
	}
}

void CInventory::SlotInit(int8 InitSlotIndex)
{
	// 기존에 있던 아이템 반납 후
	G_ObjectManager->ObjectReturn(Get(InitSlotIndex)->_GameObjectInfo.ObjectType, Get(InitSlotIndex));

	// 해당 슬롯 초기화
	auto FindInitSlot = _Items.find(InitSlotIndex);
	if (FindInitSlot != _Items.end())
	{
		(*FindInitSlot).second = (CItem*)G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_ITEM);
		(*FindInitSlot).second->_ItemInfo.ItemSlotIndex = InitSlotIndex;
	}
}
