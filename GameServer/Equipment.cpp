#include "pch.h"
#include "Equipment.h"
#include "Item.h"
#include "Player.h"

CEquipment::CEquipment()
{
	_HeadArmorItem = nullptr;
	_WearArmorItem = nullptr;
	_GloveArmorItem = nullptr;
	_BootArmorItem = nullptr;
	_WeaponItem = nullptr;
}

CEquipment::~CEquipment()
{
	
}

void CEquipment::ItemEquip(CItem* EquipItem, CPlayer* ReqEquipItemPlayer)
{
	switch (EquipItem->_ItemInfo.ItemSmallCategory)
	{
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD:
		{
			_WeaponItem = EquipItem;

			_WeaponMinDamage = _WeaponItem->_ItemInfo.ItemMinDamage;
			_WeaponMaxDamage = _WeaponItem->_ItemInfo.ItemMaxDamage;

			// ��û�� ��� ������ ���� �߿��� ���� ���� ��� ���� 
			vector<CItem*> Weapons = ReqEquipItemPlayer->_Inventory.Find(EquipItem->_ItemInfo.ItemLargeCategory);
			for (CItem* WeaponItem : Weapons)
			{
				if (WeaponItem->_ItemInfo.ItemIsEquipped == true)
				{
					WeaponItem->_ItemInfo.ItemIsEquipped = false;
					break;
				}
			}			
		}
		break;		
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER:
		{
			_HeadArmorItem = EquipItem;

			_HeadArmorDefence = _HeadArmorItem->_ItemInfo.ItemDefence;

			// ��û�� ��� ������ �Ӹ� �� �߿��� ���� ���� ��� ����
			vector<CItem*> HeadArmors = ReqEquipItemPlayer->_Inventory.Find(EquipItem->_ItemInfo.ItemLargeCategory);
			for (CItem* HeadArmor : HeadArmors)
			{
				if (HeadArmor->_ItemInfo.ItemIsEquipped == true)
				{
					HeadArmor->_ItemInfo.ItemIsEquipped = false;
					break;
				}
			}
		}		
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_WOOD:
		{
			_WearArmorItem = EquipItem;

			_WearArmorDefence = _WearArmorItem->_ItemInfo.ItemDefence;

			// ��û�� ��� ������ ���� �� �߿��� ���� ���� ��� ����
			vector<CItem*> WearArmors = ReqEquipItemPlayer->_Inventory.Find(EquipItem->_ItemInfo.ItemLargeCategory);
			for (CItem* WearArmor : WearArmors)
			{
				if (WearArmor->_ItemInfo.ItemIsEquipped == true)
				{
					WearArmor->_ItemInfo.ItemIsEquipped = false;
					break;
				}
			}
		}		
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER:
		{
			_BootArmorItem = EquipItem;

			_BootArmorDefence = _BootArmorItem->_ItemInfo.ItemDefence;

			// ��û�� ��� ������ �Ź� �� �߿��� ���� ���� ��� ����
			vector<CItem*> BootArmors = ReqEquipItemPlayer->_Inventory.Find(EquipItem->_ItemInfo.ItemLargeCategory);
			for (CItem* BootArmor : BootArmors)
			{
				if (BootArmor->_ItemInfo.ItemIsEquipped == true)
				{
					BootArmor->_ItemInfo.ItemIsEquipped = false;
					break;
				}
			}
		}		
		break;	
	}

	EquipItem->_ItemInfo.ItemIsEquipped = true;
}
