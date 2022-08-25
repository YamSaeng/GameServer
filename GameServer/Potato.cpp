#include "pch.h"
#include "Potato.h"
#include "DataManager.h"
#include "ObjectManager.h"
#include <atlbase.h>

CPotato::CPotato()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_CROP_POTATO;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

	st_ItemInfo* FindCropItemInfo = G_Datamanager->FindItemData(en_SmallItemCategory::ITEM_SMALL_CATEGORY_CROP_FRUIT_POTATO);

	_GameObjectInfo.ObjectName = FindCropItemInfo->ItemName;

	_GameObjectInfo.ObjectStatInfo.MaxHP = FindCropItemInfo->MaxHP;
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

		G_ObjectManager->ObjectItemSpawn(_Channel, Attacker->_GameObjectInfo.ObjectId,
			Attacker->_GameObjectInfo.ObjectType,
			_GameObjectInfo.ObjectPositionInfo.CollisionPosition,
			_GameObjectInfo.ObjectPositionInfo.Position,
			_GameObjectInfo.ObjectType,
			en_GameObjectType::OBJECT_CROP_POTATO);
	}

	return IsDead;
}
