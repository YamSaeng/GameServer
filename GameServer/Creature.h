#pragma once
#include "GameObject.h"
#include "InventoryManager.h"
#include "EquipmentBox.h"

class CCreature : public CGameObject
{
public:
	CCreature();
	~CCreature();

	//--------------------------------------------
	// ���� ��ȯ
	//--------------------------------------------
	CInventoryManager* GetInventoryManager();
	CEquipmentBox* GetEquipment();

	//--------------------------------------------
	// NPC �ʱ�ȭ
	//--------------------------------------------
	void NPCInit(en_NonPlayerType NonPlayerType);

	// ���ӱ� ��ų
	CSkill* _ComboSkill;
protected:
	CInventoryManager _Inventory;
	CEquipmentBox _Equipment;
};

