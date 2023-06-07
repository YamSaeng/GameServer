#pragma once
#include "GameObjectInfo.h"

class CItem;
class CPlayer;

class CEquipmentBox
{
public:
	int32 _WeaponMinDamage;
	int32 _WeaponMaxDamage;
	int32 _HeadArmorDefence;
	int32 _WearArmorDefence;
	int32 _BootArmorDefence;

	CEquipmentBox();
	~CEquipmentBox();

	CItem* ItemOnEquipment(CItem* OnEquipItem);
	CItem* ItemOffEquipment(en_EquipmentParts OffEquipmentParts);	

	CItem* GetEquipmentParts(en_EquipmentParts EquipmentPart);

	map<en_EquipmentParts, CItem*> GetEquipments();
private:
	map<en_EquipmentParts, CItem*> _EquipmentParts;	
};

