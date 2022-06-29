#include "pch.h"
#include "Potato.h"
#include "DataManager.h"
#include "ObjectManager.h"
#include <atlbase.h>

CPotato::CPotato()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_CROP_POTATO;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

	st_ItemData* FindCropData = G_Datamanager->FindItemData(en_SmallItemCategory::ITEM_SMALL_CATEGORY_CROP_FRUIT_POTATO);

	_GameObjectInfo.ObjectName = (LPWSTR)CA2W(FindCropData->ItemName.c_str());

	_GameObjectInfo.ObjectStatInfo.MaxHP = FindCropData->ItemCraftingMaxHP;
	_GameObjectInfo.ObjectStatInfo.HP = _GameObjectInfo.ObjectStatInfo.MaxHP;

	_GameObjectInfo.ObjectWidth = 1;
	_GameObjectInfo.ObjectHeight = 1;
}

CPotato::~CPotato()
{
}

bool CPotato::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	bool IsDead = CCrop::OnDamaged(Attacker, Damage);

	if (IsDead == true)
	{
		_DeadReadyTick = GetTickCount64() + 300;

		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::READY_DEAD;

		CMessage* ResChangeStatePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId,
			_GameObjectInfo.ObjectPositionInfo.MoveDir,
			_GameObjectInfo.ObjectType,
			_GameObjectInfo.ObjectPositionInfo.State);
		G_ObjectManager->GameServer->SendPacketFieldOfView(this, ResChangeStatePacket);
		ResChangeStatePacket->Free();

		G_ObjectManager->ObjectItemSpawn(Attacker->_GameObjectInfo.ObjectId,
			Attacker->_GameObjectInfo.ObjectType,
			_GameObjectInfo.ObjectPositionInfo.CollisionPosition,
			_GameObjectInfo.ObjectType,
			en_GameObjectType::OBJECT_CROP_POTATO);
	}

	return IsDead;
}
