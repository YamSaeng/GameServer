#pragma once
#include "GameObject.h"

enum en_ItemType
{
	ITEM_TYPE_NONE = 0,

	ITEM_TYPE_WEAPON_SWORD = 1,

	ITEM_TYPE_ARMOR_ARMOR = 100,
	ITEM_TYPE_ARMOR_HELMET = 101,

	ITEM_TYPE_CONSUMABLE_POTION = 200,

	ITEM_TYPE_LEATHER = 2000,
	ITEM_TYPE_SLIMEGEL = 2001,
	ITEM_TYPE_BRONZE_COIN = 2002
};

struct st_ItemInfo
{
	int64 ItemDBId;				// 아이템 DB에 저장되어 있는 ID	
	int32 Count;				// 개수
	int32 SlotNumber;			// 슬롯 번호
	bool IsEquipped;			// 아이템을 착용할 수 있는지
	en_ItemType ItemType;		// 아이템 타입
	wstring ItemName;			// 아이템 이름
	wstring ThumbnailImagePath; // 이미지 경로
};

class CItem : public CGameObject
{
public:	
	st_ItemInfo _ItemInfo;	
	st_Vector2Int _OwnerPosition;

	CItem();
};

class CWeapon : public CItem
{

};

class CMaterial : public CItem
{
public:
	CMaterial();
};