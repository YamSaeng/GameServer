#include "pch.h"
#include "GeneralMerchantNPC.h"
#include "DataManager.h"
#include "ObjectManager.h"

CGeneralMerchantNPC::CGeneralMerchantNPC()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_NON_PLAYER;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

	_NonPlayerType = en_NonPlayerType::NON_PLAYER_CHARACTER_일반_상인;	

	switch (_NonPlayerType)
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
	}	
}

CGeneralMerchantNPC::~CGeneralMerchantNPC()
{

}
