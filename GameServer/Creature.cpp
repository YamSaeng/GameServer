#include "pch.h"
#include "Creature.h"
#include "DataManager.h"
#include "ObjectManager.h"

CCreature::CCreature()
{
	
}

CCreature::~CCreature()
{
}

CInventoryManager* CCreature::GetInventoryManager()
{
    return &_Inventory;
}

CEquipmentBox* CCreature::GetEquipment()
{
	return &_Equipment;
}

void CCreature::NPCInit(en_NonPlayerType NonPlayerType)
{
	switch (NonPlayerType)
	{
	case en_NonPlayerType::NON_PLAYER_CHARACTER_일반_상인:
		{
			for (auto MerchantItemIter : G_Datamanager->_GeneralMerchantItems)
			{
				CItem* MerchantItem = G_ObjectManager->ItemCreate((en_SmallItemCategory)MerchantItemIter.first);
				_Inventory.DBItemInsertItem(0, MerchantItem);
			}
		}		
		break;
	default:
		break;
	}
}

void CCreature::NPCInventoryCreate()
{
	if (_Inventory.GetInventoryManager() == nullptr)
	{
		_Inventory.InventoryCreate(1, 10, 10);
	}

	auto FindDropItem = G_Datamanager->_DropItems.find(_GameObjectInfo.ObjectType);
	if (FindDropItem != G_Datamanager->_DropItems.end())
	{
		en_SmallItemCategory DropItemCategory;
		int16 DropItemCount = 0;

		random_device RD;
		mt19937 Gen(RD());
		uniform_real_distribution<float> RandomDropPoint(0, 1); // 0.0 ~ 1.0
		float RandomPoint = 100 * RandomDropPoint(Gen);

		int32 Sum = 0;

		vector<st_DropData> DropItems = (*FindDropItem).second;
		for (st_DropData DropItem : DropItems)
		{
			Sum += DropItem.Probability;

			if (Sum >= RandomPoint)
			{
				uniform_int_distribution<int> RandomDropItemCount(DropItem.MinCount, DropItem.MaxCount);
				DropItemCount = RandomDropItemCount(Gen);
				DropItemCategory = DropItem.DropItemSmallCategory;

				CItem* NewInventoryItem = G_ObjectManager->ItemCreate(DropItem.DropItemSmallCategory);
				if (NewInventoryItem != nullptr)
				{
					NewInventoryItem->_ItemInfo.ItemCount = DropItemCount;

					NewInventoryItem->_GameObjectInfo.ObjectType = NewInventoryItem->_ItemInfo.ItemObjectType;
					NewInventoryItem->_GameObjectInfo.ObjectName = NewInventoryItem->_ItemInfo.ItemName;
					NewInventoryItem->_GameObjectInfo.OwnerObjectId = _GameObjectInfo.ObjectId;
					NewInventoryItem->_GameObjectInfo.OwnerObjectType = _GameObjectInfo.ObjectType;

					_Inventory.InsertItem(0, NewInventoryItem);
				}
			}
		}
	}	
}
