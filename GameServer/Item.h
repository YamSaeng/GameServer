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
	int64 ItemDBId;				// ������ DB�� ����Ǿ� �ִ� ID	
	int32 Count;				// ����
	int32 SlotNumber;			// ���� ��ȣ
	bool IsEquipped;			// �������� ������ �� �ִ���
	en_ItemType ItemType;		// ������ Ÿ��
	wstring ItemName;			// ������ �̸�
	wstring ThumbnailImagePath; // �̹��� ���
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