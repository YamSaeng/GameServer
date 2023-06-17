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
	
protected:
	CInventoryManager _Inventory;
	CEquipmentBox _Equipment;
};

