#pragma once

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

	void ItemEquip(CItem* EquipItem, CPlayer* ReqEquipItemPlayer);
private:
	enum en_Equipment
	{
		EQUIPMENT_PARTS_MAX = 6
	};

	// �������� ��� ������
	CItem* _EquipmentParts[en_Equipment::EQUIPMENT_PARTS_MAX];	
};

