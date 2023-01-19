#include "pch.h"
#include "Creature.h"
#include "DataManager.h"
#include "ObjectManager.h"

CInventoryManager* CCreature::GetInventoryManager()
{
    return &_Inventory;
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
