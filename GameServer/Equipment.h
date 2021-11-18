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
	// 장비 창이 관리중인 아이템
	//----------------------------
	
	// 방어구
	// 머리
	CItem* _HeadArmorItem;
	// 갑옷
	CItem* _WearArmorItem;
	// 장갑
	CItem* _GloveArmorItem;
	// 신발
	CItem* _BootArmorItem;

	// 무기
	CItem* _WeaponItem;
};

