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
	//----------------------------
	// ��� â�� �������� ������
	//----------------------------
	
	// ��
	// �Ӹ�
	CItem* _HeadArmorItem;
	// ����
	CItem* _WearArmorItem;
	// �尩
	CItem* _GloveArmorItem;
	// �Ź�
	CItem* _BootArmorItem;

	// ����
	CItem* _WeaponItem;
};

