#pragma once
#include "GameObjectInfo.h"

class CItem;
class CPlayer;

class CEquipment
{
public:
	int32 _WeaponMinDamage;
	int32 _WeaponMaxDamage;
	int32 _HeadArmorDefence;
	int32 _WearArmorDefence;
	int32 _BootArmorDefence;

	CEquipment();
	~CEquipment();

	CItem* ItemOnEquipment(CItem* OnEquipItem);
	CItem* ItemOffEquipment(en_EquipmentParts OffEquipmentParts);	
private:
	enum en_Equipment
	{
		EQUIPMENT_PARTS_MAX = 6
	};

	// 관리중인 장비 아이템
	CItem* _EquipmentParts[en_Equipment::EQUIPMENT_PARTS_MAX];	
};

