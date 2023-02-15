#include "pch.h"
#include "EquipmentBox.h"
#include "Item.h"
#include "Player.h"

CEquipmentBox::CEquipmentBox()
{
	_WeaponMinDamage = 0;
	_WeaponMaxDamage = 0;
	_HeadArmorDefence = 0;
	_WearArmorDefence = 0;
	_BootArmorDefence = 0;
}

CEquipmentBox::~CEquipmentBox()
{

}

CItem* CEquipmentBox::ItemOnEquipment(CItem* OnEquipItem)
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

CItem* CEquipmentBox::ItemOffEquipment(en_EquipmentParts OffEquipmentParts)
{	
	CItem* ReturnEquipItem = nullptr;
	_EquipmentParts[OffEquipmentParts]->_ItemInfo.ItemIsEquipped = false;

	ReturnEquipItem = _EquipmentParts[OffEquipmentParts];

	_EquipmentParts[OffEquipmentParts] = nullptr;

	return ReturnEquipItem;
}

CItem* CEquipmentBox::GetEquipmentParts(en_EquipmentParts EquipmentPart)
{
	auto EquipmentIter = _EquipmentParts.find(EquipmentPart);
	if (EquipmentIter != _EquipmentParts.end())
	{
		return (*EquipmentIter).second;
	}
	else
	{
		return nullptr;
	}	
}
