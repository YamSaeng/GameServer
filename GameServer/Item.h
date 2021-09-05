#pragma once
#include "GameObject.h"

enum class en_ItemType : int16
{
	ITEM_TYPE_NONE = 0,

	ITEM_TYPE_WEAPON_SWORD = 1,

	ITEM_TYPE_ARMOR_ARMOR = 100,
	ITEM_TYPE_ARMOR_HELMET = 101,

	ITEM_TYPE_CONSUMABLE_POTION = 200,

	ITEM_TYPE_LEATHER = 2000,
	ITEM_TYPE_SLIMEGEL = 2001,
	ITEM_TYPE_BRONZE_COIN = 2002,
	ITEM_TYPE_SLIVER_COIN = 2003,
	ITEM_TYPE_GOLD_COIN = 2004
};

struct st_ItemInfo
{
	int64 ItemDBId;				// 아이템 DB에 저장되어 있는 ID		
	en_ItemType ItemType;		// 아이템 타입
	wstring ItemName;			// 아이템 이름
	int16 ItemCount;			// 개수
	wstring ThumbnailImagePath; // 이미지 경로
	bool IsEquipped;			// 아이템을 착용할 수 있는지	
	int8 SlotIndex;				// 슬롯 번호
};

class CItem : public CGameObject
{
public:	
	st_ItemInfo _ItemInfo;	
	st_Vector2Int _SpawnPosition;

	int64 _OwnerObjectId;
	en_GameObjectType _OwnerObjectType;

	CItem();

	virtual void Update() override;
	void SetDestoryTime(int32 DestoryTime);
	void ItemSetTarget(en_GameObjectType TargetType, int64 TargetDBId);
protected:
	int64 _DestroyTime;

	virtual void UpdateIdle();
};

class CWeapon : public CItem
{

};

class CMaterial : public CItem
{
public:
	CMaterial();
};