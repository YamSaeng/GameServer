#include "pch.h"
#include "Item.h"
#include "ObjectManager.h"
#include "GameServerMessage.h"

CItem::CItem()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_ITEM;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

	_FieldOfViewDistance = 10;
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
	_ItemInfo.ItemThumbnailImagePath = L"";
	_ItemInfo.ItemIsEquipped = false;
}

void CItem::UpdateIdle()
{
	if (_Owner && _Owner->_GameObjectInfo.ObjectPositionInfo.CollisionPosition == _GameObjectInfo.ObjectPositionInfo.CollisionPosition)
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

void CArchitecture::UpdateIdle()
{
	CItem::UpdateIdle();
}

CArchitecture::CArchitecture()
{
	
}

CArchitecture::~CArchitecture()
{
}
