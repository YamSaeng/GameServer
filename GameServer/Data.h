#pragma once
#include "Item.h"

struct st_ItemData
{
	int32 _DataSheetId;
	string _Name;
	en_ItemType _ItemType;
	string _ImagePath;
};

struct st_WeaponData : public st_ItemData
{
public:	
	int32 _Damage;
};

struct st_ArmorData : public st_ItemData
{
	int32 _Defence;
};

struct st_ConsumableData : public st_ItemData
{
	int32 _MaxCount;
};

struct st_StatusData
{
	int32 Level;
	int32 HP;
	int32 MaxHP;
	int32 Attack;
	float Speed;
};