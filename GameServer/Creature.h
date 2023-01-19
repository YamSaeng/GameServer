#pragma once
#include "GameObject.h"
#include "InventoryManager.h"

class CCreature : public CGameObject
{
public:
	//--------------------------------------------
	// ���� ��ȯ
	//--------------------------------------------
	CInventoryManager* GetInventoryManager();

	//--------------------------------------------
	// NPC �ʱ�ȭ
	//--------------------------------------------
	void NPCInit(en_NonPlayerType NonPlayerType);
protected:
	CInventoryManager _Inventory;
};

