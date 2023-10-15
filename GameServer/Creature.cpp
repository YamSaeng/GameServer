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
	if (_Inventory.GetInventoryManager().size() == 0)
	{
		_Inventory.InventoryCreate(1, 10, 10);		

		auto FindDropItem = G_Datamanager->_DropItems.find(_GameObjectInfo.ObjectType);
		if (FindDropItem != G_Datamanager->_DropItems.end())
		{
			en_SmallItemCategory DropItemCategory;
			int16 DropItemCount = 0;

			float RandomPoint = 100 * Math::RandomNumberFloat(0, 1.0f);

			int32 Sum = 0;

			vector<st_DropData> DropItems = (*FindDropItem).second;
			for (st_DropData DropItem : DropItems)
			{
				Sum += DropItem.Probability;

				if (Sum >= RandomPoint)
				{					
					DropItemCount = Math::RandomNumberInt(DropItem.MinCount, DropItem.MaxCount);
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

		auto FindDropMoneyData = G_Datamanager->_DropMoneys.find(_GameObjectInfo.ObjectStatInfo.Level);
		if (FindDropMoneyData != G_Datamanager->_DropMoneys.end())
		{
			Vector2Int Money = (*FindDropMoneyData).second;			

			int RandomMoneyPoint = Math::RandomNumberInt(Money.X, Money.Y);
			_Inventory.InsertMoney(0, RandomMoneyPoint);
		}
	}	
}
