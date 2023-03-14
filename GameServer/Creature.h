#pragma once
#include "GameObject.h"
#include "InventoryManager.h"

class CCreature : public CGameObject
{
public:
	CCreature();
	~CCreature();

	//--------------------------------------------
	// 가방 반환
	//--------------------------------------------
	CInventoryManager* GetInventoryManager();

	//--------------------------------------------
	// NPC 초기화
	//--------------------------------------------
	void NPCInit(en_NonPlayerType NonPlayerType);

	// 연속기 스킬
	CSkill* _ComboSkill;
protected:
	CInventoryManager _Inventory;
};

