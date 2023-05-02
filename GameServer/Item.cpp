#include "pch.h"
#include "Item.h"
#include "NetworkManager.h"
#include "GameServerMessage.h"
#include <math.h>

CItem::CItem()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_ITEM;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

	_ItemState = en_ItemState::ITEM_IDLE;	

	_FieldOfViewDistance = 10;

	_ChaseWaitTime = 0;	

	_RectCollision = nullptr;
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
	if (_Owner && _Owner->_NetworkState == en_ObjectNetworkState::LEAVE)
	{
		_Owner = nullptr;
	}

	// 상태에 따라 Update문 따로 호출
	switch (_GameObjectInfo.ObjectPositionInfo.State)
	{
	case en_CreatureState::IDLE:
		UpdateIdle();
		break;
	case en_CreatureState::MOVING:
		switch (_ItemState)
		{
		case en_ItemState::ITEM_IDLE:			
			break;
		case en_ItemState::ITEM_READY_MOVE:
			UpdateReadyMoving();
			break;
		case en_ItemState::ITEM_MOVE:
			UpdateMoving();
			break;		
		}		
		break;
	default:
		break;
	}
}

void CItem::CraftingStart()
{
	_ItemCrafting = en_ItemCrafting::ITEM_CRAFTING_START;

	_CraftingTick = _ItemInfo.ItemCraftingTime + GetTickCount64();

	_ItemInfo.ItemCraftingRemainTime = _CraftingTick - GetTickCount64();
}

void CItem::CraftingStop()
{
	_ItemCrafting = en_ItemCrafting::ITEM_CRAFTING_STOP;

	_CraftingTick = 0;

	_ItemInfo.ItemCraftingRemainTime = 0;
}

void CItem::SetDestoryTime(int32 DestoryTime)
{
	_DestroyTime += (GetTickCount64() + DestoryTime);
}

void CItem::ItemSetTarget(en_GameObjectType TargetType, int64 TargetDBId)
{
	if (_Channel != nullptr)
	{
		_Owner = _Channel->FindChannelObject(TargetDBId, TargetType);
	}	
}

void CItem::UpdateIdle()
{
	if (_Owner && _Owner->_GameObjectInfo.ObjectPositionInfo.CollisionPosition == _GameObjectInfo.ObjectPositionInfo.CollisionPosition)
	{		
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::DEAD;		
	}	

	if (_Owner != nullptr)
	{
		float TargetDistance = Vector2::Distance(_Owner->_GameObjectInfo.ObjectPositionInfo.Position, _GameObjectInfo.ObjectPositionInfo.Position);
		if (TargetDistance <= 5.0f)
		{
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;
			_ItemState = en_ItemState::ITEM_READY_MOVE;			
		}
	}	
}

void CItem::UpdateReadyMoving()
{
	_ChaseWaitTime = GetTickCount64() + 1000;

	vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = _Channel->GetMap()->GetFieldAroundPlayers(this, false);

	CMessage* S2CItemMoveMessage = G_NetworkManager->GetGameServer()->MakePacketItemMove(_GameObjectInfo);
	G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, S2CItemMoveMessage);
	S2CItemMoveMessage->Free();

	_ItemState = en_ItemState::ITEM_MOVE;
}

void CItem::UpdateMoving()
{
	if (_ChaseWaitTime < GetTickCount64())
	{
		float TargetDistance = Vector2::Distance(_Owner->_GameObjectInfo.ObjectPositionInfo.Position, _GameObjectInfo.ObjectPositionInfo.Position);

		if (TargetDistance > 0.5f)
		{
			float num = _Owner->_GameObjectInfo.ObjectPositionInfo.Position.X - _GameObjectInfo.ObjectPositionInfo.Position.X;
			float num2 = _Owner->_GameObjectInfo.ObjectPositionInfo.Position.Y - _GameObjectInfo.ObjectPositionInfo.Position.Y;

			float num4 = num * num + num2 * num2;

			float num5 = (float)sqrt(num4);

			_GameObjectInfo.ObjectPositionInfo.Position.X = _GameObjectInfo.ObjectPositionInfo.Position.X + num / num5 * _Owner->_GameObjectInfo.ObjectStatInfo.Speed * 2.0f * 0.02f;
			_GameObjectInfo.ObjectPositionInfo.Position.Y = _GameObjectInfo.ObjectPositionInfo.Position.Y + num2 / num5 * _Owner->_GameObjectInfo.ObjectStatInfo.Speed * 2.0f * 0.02f;

			Vector2Int CollisionPosition;
			CollisionPosition.X = _GameObjectInfo.ObjectPositionInfo.Position.X;
			CollisionPosition.Y = _GameObjectInfo.ObjectPositionInfo.Position.Y;

			if (CollisionPosition.X != _GameObjectInfo.ObjectPositionInfo.CollisionPosition.X
				|| CollisionPosition.Y != _GameObjectInfo.ObjectPositionInfo.CollisionPosition.Y)
			{
				_Channel->GetMap()->ApplyMove(this, CollisionPosition, false, false);
			}			
		}
		else
		{
			st_GameObjectJob* ItemSaveJob = G_NetworkManager->GetGameServer()->MakeGameObjectJobItemSave(this);
			_Owner->_GameObjectJobQue.Enqueue(ItemSaveJob);

			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::DEAD;				
		}
	}	
}

CWeaponItem::CWeaponItem()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_ITEM_WEAPON;
}

CWeaponItem::~CWeaponItem()
{
}

void CWeaponItem::UpdateIdle()
{
	CItem::UpdateIdle();
}

CArmorItem::CArmorItem()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_ITEM_ARMOR;
}

CArmorItem::~CArmorItem()
{
}

void CArmorItem::UpdateIdle()
{
	CItem::UpdateIdle();
}

CMaterialItem::CMaterialItem()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_ITEM_MATERIAL;
}

CMaterialItem::~CMaterialItem()
{
}

void CMaterialItem::UpdateIdle()
{
	CItem::UpdateIdle();	
}

void CArchitectureItem::UpdateIdle()
{
	CItem::UpdateIdle();
}

CArchitectureItem::CArchitectureItem()
{
	
}

CArchitectureItem::~CArchitectureItem()
{
}

void CCropItem::UpdateIdle()
{
	CItem::UpdateIdle();
}

CCropItem::CCropItem()
{
}

CCropItem::~CCropItem()
{
}

void CToolItem::UpdateIdle()
{
}

CToolItem::CToolItem()
{
}

CToolItem::~CToolItem()
{
}
