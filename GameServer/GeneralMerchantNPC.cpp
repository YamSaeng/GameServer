#include "pch.h"
#include "GeneralMerchantNPC.h"
#include "DataManager.h"
#include "ObjectManager.h"
#include "RectCollision.h"

CGeneralMerchantNPC::CGeneralMerchantNPC()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_NON_PLAYER_GENERAL_MERCHANT;	

	_NonPlayerType = en_NonPlayerType::NON_PLAYER_CHARACTER_일반_상인;		

	_GameObjectInfo.ObjectName = L"잡화 상인";
}

CGeneralMerchantNPC::~CGeneralMerchantNPC()
{

}

void CGeneralMerchantNPC::Update()
{
	if (_RectCollision != nullptr)
	{
		_RectCollision->Update();
	}
}

void CGeneralMerchantNPC::MerchantNPCInit()
{
	GetInventoryManager()->InventoryCreate(1, (int8)en_InventoryManager::INVENTORY_DEFAULT_WIDH_SIZE, (int8)en_InventoryManager::INVENTORY_DEFAULT_HEIGHT_SIZE);

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
