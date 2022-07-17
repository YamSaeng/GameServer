#include "pch.h"
#include "Item.h"
#include "ObjectManager.h"
#include "GameServerMessage.h"
#include <math.h>

CItem::CItem()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_ITEM;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

	_FieldOfViewDistance = 10;

	_ChaseWaitTime = 0;
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
		UpdateMoving();
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

void CItem::Init()
{
	_ItemInfo.ItemDBId = 0;
	_ItemInfo.ItemIsQuickSlotUse = false;
	_ItemInfo.Width = 0;
	_ItemInfo.Height = 0;
	_ItemInfo.Rotated = false;
	_ItemInfo.TileGridPositionX = 0;
	_ItemInfo.TileGridPositionY = 0;
	_ItemInfo.ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_NONE;
	_ItemInfo.ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_NONE;
	_ItemInfo.ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_NONE;
	_ItemInfo.ItemName = L"";
	_ItemInfo.ItemExplain = L"";
	_ItemInfo.ItemMinDamage = 0;
	_ItemInfo.ItemMaxDamage = 0;
	_ItemInfo.ItemDefence = 0;
	_ItemInfo.ItemMaxCount = 0;
	_ItemInfo.ItemCount = 0;	
	_ItemInfo.ItemIsEquipped = false;
}

void CItem::UpdateIdle()
{
	if (_Owner && _Owner->_GameObjectInfo.ObjectPositionInfo.CollisionPosition == _GameObjectInfo.ObjectPositionInfo.CollisionPosition)
	{		
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::DEAD;		
	}	

	if (_Owner != nullptr)
	{
		float TargetDistance = st_Vector2::Distance(_Owner->_GameObjectInfo.ObjectPositionInfo.Position, _GameObjectInfo.ObjectPositionInfo.Position);
		if (TargetDistance <= 5.0f)
		{
			_ChaseWaitTime = GetTickCount64() + 1000;

			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;
		}
	}	
}

void CItem::UpdateMoving()
{
	if (_ChaseWaitTime < GetTickCount64())
	{
		float TargetDistance = st_Vector2::Distance(_Owner->_GameObjectInfo.ObjectPositionInfo.Position, _GameObjectInfo.ObjectPositionInfo.Position);

		if (TargetDistance > 0.5f)
		{
			float num = _Owner->_GameObjectInfo.ObjectPositionInfo.Position._X - _GameObjectInfo.ObjectPositionInfo.Position._X;
			float num2 = _Owner->_GameObjectInfo.ObjectPositionInfo.Position._Y - _GameObjectInfo.ObjectPositionInfo.Position._Y;

			float num4 = num * num + num2 * num2;

			float num5 = (float)sqrt(num4);

			_GameObjectInfo.ObjectPositionInfo.Position._X = _GameObjectInfo.ObjectPositionInfo.Position._X + num / num5 * _Owner->_GameObjectInfo.ObjectStatInfo.Speed * 2.0f * 0.02f;
			_GameObjectInfo.ObjectPositionInfo.Position._Y = _GameObjectInfo.ObjectPositionInfo.Position._Y + num2 / num5 * _Owner->_GameObjectInfo.ObjectStatInfo.Speed * 2.0f * 0.02f;

			st_Vector2Int CollisionPosition;
			CollisionPosition._X = _GameObjectInfo.ObjectPositionInfo.Position._X;
			CollisionPosition._Y = _GameObjectInfo.ObjectPositionInfo.Position._Y;

			if (CollisionPosition._X != _GameObjectInfo.ObjectPositionInfo.CollisionPosition._X
				|| CollisionPosition._Y != _GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y)
			{
				_Channel->GetMap()->ApplyPositionUpdateItem(this, CollisionPosition);
			}

			CMessage* S2CMoveMessage = G_ObjectManager->GameServer->MakePacketItemMove(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectPositionInfo);
			G_ObjectManager->GameServer->SendPacket(((CPlayer*)_Owner)->_SessionId, S2CMoveMessage);
			S2CMoveMessage->Free();
		}
		else
		{
			st_GameObjectJob* ItemSaveJob = G_ObjectManager->GameServer->MakeGameObjectJobItemSave(this);
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
