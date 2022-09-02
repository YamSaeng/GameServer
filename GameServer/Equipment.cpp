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

CItem* CEquipment::ItemOnEquipment(CItem* OnEquipItem)
{
	CItem* ReturnEquipItem = nullptr;

	if (_EquipmentParts[OnEquipItem->_ItemInfo.ItemEquipmentPart] != nullptr && _EquipmentParts[OnEquipItem->_ItemInfo.ItemEquipmentPart]->_ItemInfo.ItemIsEquipped == true)
	{
		ReturnEquipItem = _EquipmentParts[OnEquipItem->_ItemInfo.ItemEquipmentPart];
		_EquipmentParts[OnEquipItem->_ItemInfo.ItemEquipmentPart]->_ItemInfo.ItemIsEquipped = false;
	}

	_EquipmentParts[OnEquipItem->_ItemInfo.ItemEquipmentPart] = OnEquipItem;
	_EquipmentParts[OnEquipItem->_ItemInfo.ItemEquipmentPart]->_ItemInfo.ItemIsEquipped = true;

	return ReturnEquipItem;
}

CItem* CEquipment::ItemOffEquipment(en_EquipmentParts OffEquipmentParts)
{	
	CItem* ReturnEquipItem = nullptr;
	_EquipmentParts[OffEquipmentParts]->_ItemInfo.ItemIsEquipped = false;

	ReturnEquipItem = _EquipmentParts[OffEquipmentParts];

	_EquipmentParts[OffEquipmentParts] = nullptr;

	return ReturnEquipItem;
}

CItem** CEquipment::GetEquipmentParts()
{
	return _EquipmentParts;
}
