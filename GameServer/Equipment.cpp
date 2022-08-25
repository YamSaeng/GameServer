#include "pch.h"
#include "Equipment.h"
#include "Item.h"
#include "Player.h"

CEquipment::CEquipment()
{
	_WeaponMinDamage = 0;
	_WeaponMaxDamage = 0;
	_HeadArmorDefence = 0;
	_WearArmorDefence = 0;
	_BootArmorDefence = 0;

}

CEquipment::~CEquipment()
{

}

void CEquipment::ItemEquip(CItem* EquipItem, CPlayer* ReqEquipItemPlayer)
{
	switch (EquipItem->_ItemInfo.ItemSmallCategory)
	{
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD:
		_EquipmentParts[en_EquipmentParts::EQUIPMENT_PARTS_LEFT_HAND]->_ItemInfo.ItemIsEquipped = false;
		_EquipmentParts[en_EquipmentParts::EQUIPMENT_PARTS_LEFT_HAND] = EquipItem;

		//_WeaponMinDamage = _WeaponItem->_ItemInfo.ItemMinDamage;
		//_WeaponMaxDamage = _WeaponItem->_ItemInfo.ItemMaxDamage;			
		break;
	case en_SmallItemCategory::ITEM_SAMLL_CATEGORY_WEAPON_WOOD_SHIELD:
		_EquipmentParts[en_EquipmentParts::EQUIPMENT_PARTS_RIGHT_HAND]->_ItemInfo.ItemIsEquipped = false;
		_EquipmentParts[en_EquipmentParts::EQUIPMENT_PARTS_RIGHT_HAND] = EquipItem;
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER:
		_EquipmentParts[en_EquipmentParts::EQUIPMENT_PARTS_HEAD]->_ItemInfo.ItemIsEquipped = false;
		_EquipmentParts[en_EquipmentParts::EQUIPMENT_PARTS_HEAD] = EquipItem;

		//_HeadArmorDefence = _HeadArmorItem->_ItemInfo.ItemDefence;							
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_WOOD:
		_EquipmentParts[en_EquipmentParts::EQUIPMENT_PARTS_BODY]->_ItemInfo.ItemIsEquipped = false;
		_EquipmentParts[en_EquipmentParts::EQUIPMENT_PARTS_BODY] = EquipItem;

		//_WearArmorDefence = _WearArmorItem->_ItemInfo.ItemDefence;							
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER:
		_EquipmentParts[en_EquipmentParts::EQUIPMENT_PARTS_BOOT]->_ItemInfo.ItemIsEquipped = false;		
		_EquipmentParts[en_EquipmentParts::EQUIPMENT_PARTS_BOOT] = EquipItem;

		//_BootArmorDefence = _BootArmorItem->_ItemInfo.ItemDefence;							
		break;
	}

	EquipItem->_ItemInfo.ItemIsEquipped = true;
}
