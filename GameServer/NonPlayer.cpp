#include "pch.h"
#include "NonPlayer.h"
#include "DataManager.h"
#include "ObjectManager.h"

CNonPlayer::CNonPlayer()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_NON_PLAYER;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

	_FieldOfViewDistance = 10;
}

CNonPlayer::~CNonPlayer()
{

}

void CNonPlayer::Update()
{
}

bool CNonPlayer::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	return false;
}

void CNonPlayer::Init(en_NonPlayerType NonPlayerType)
{
	switch (NonPlayerType)
	{	
	case en_NonPlayerType::NON_OBJECT_PLAYER_GENERAL_MERCHANT:
		{
			for (auto MerchantItemIter : G_Datamanager->_GeneralMerchantItems)
			{
				CItem* MerchantItem = G_ObjectManager->ItemCreate((en_SmallItemCategory)MerchantItemIter.first);
				_InventoryManager.DBItemInsertItem(0, MerchantItem);
			}			
		}
		break;	
	}
}
