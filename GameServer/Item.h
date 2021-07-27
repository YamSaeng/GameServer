#pragma once

struct st_ItemInfo
{
	int32 ItemDBId;
	int32 DataSheetId;
	int32 Count;
	int32 Slot;
	bool IsEquipped;
};

enum en_ItemType
{
	ITEM_TYPE_NONE,

	ITEM_TYPE_WEAPON_SWORD,
	ITEM_TYPE_WEAPON_BOW,

	ITEM_TYPE_ARMOR_HELMET,
	ITEM_TYPE_ARMOR_ARMOR,
	ITEM_TYPE_ARMOR_BOOTS,
	
	ITEM_TYPE_CONSUMABLE_POTION
};

class CItem
{
public:	
};

