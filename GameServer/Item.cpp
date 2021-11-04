#include "pch.h"
#include "Item.h"
#include "ObjectManager.h"
#include "GameServerMessage.h"

CItem::CItem()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_ITEM;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
}

CItem::~CItem()
{
	
}

void CItem::Update()
{
	if (_DestroyTime < GetTickCount64())
	{
	}

	// 타겟의 상태가 LEAVE면 타겟을 없애준다.
	if (_Target && _Target->_NetworkState == en_ObjectNetworkState::LEAVE)
	{
		_Target = nullptr;
	}

	// 상태에 따라 Update문 따로 호출
	switch (_GameObjectInfo.ObjectPositionInfo.State)
	{
	case en_CreatureState::IDLE:
		UpdateIdle();
		break;
	default:
		break;
	}
}

void CItem::SetDestoryTime(int32 DestoryTime)
{
	_DestroyTime += (GetTickCount64() + DestoryTime);
}

void CItem::ItemSetTarget(en_GameObjectType TargetType, int64 TargetDBId)
{
	_Target = G_ObjectManager->Find(TargetDBId, TargetType);
}

void CItem::UpdateIdle()
{
	if (_Target && _Target->GetCellPosition() == GetCellPosition())
	{		
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::DEAD;		
	}
}

CWeapon::CWeapon()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_ITEM_WEAPON;
}

CWeapon::~CWeapon()
{
}

void CWeapon::UpdateIdle()
{
	CItem::UpdateIdle();
}

CArmor::CArmor()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_ITEM_ARMOR;
}

CArmor::~CArmor()
{
}

void CArmor::UpdateIdle()
{
	CItem::UpdateIdle();
}

CMaterial::CMaterial()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_ITEM_MATERIAL;
}

CMaterial::~CMaterial()
{
}

void CMaterial::UpdateIdle()
{
	CItem::UpdateIdle();	
}