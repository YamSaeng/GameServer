#pragma once
#include "GameObject.h"
#include "InventoryManager.h"

class CCreature : public CGameObject
{
public:
	//--------------------------------------------
	// 가방 반환
	//--------------------------------------------
	CInventoryManager* GetInventoryManager();

	//--------------------------------------------
	// NPC 초기화
	//--------------------------------------------
	void NPCInit(en_NonPlayerType NonPlayerType);
protected:
	CInventoryManager _Inventory;
};

