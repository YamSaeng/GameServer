#pragma once
#include "GameObject.h"
#include "InventoryManager.h"

class CCreature : public CGameObject
{
public:
	CCreature();
	~CCreature();

	//--------------------------------------------
	// ���� ��ȯ
	//--------------------------------------------
	CInventoryManager* GetInventoryManager();

	//--------------------------------------------
	// NPC �ʱ�ȭ
	//--------------------------------------------
	void NPCInit(en_NonPlayerType NonPlayerType);

	// ���ӱ� ��ų
	CSkill* _ComboSkill;
protected:
	CInventoryManager _Inventory;
};

