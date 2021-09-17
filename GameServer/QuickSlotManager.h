#pragma once
#include "GameObjectInfo.h"

class CQuickSlotBar;

class CQuickSlotManager
{
public:
	CQuickSlotManager();
	~CQuickSlotManager();

	// Äü½½·Ô¹Ù 3°³ »ý¼º
	void Init();

	// Äü½½·Ô¹Ù¿¡ Äü½½·Ô µî·Ï
	void AddQuickSlotBarSlot(st_QuickSlotBarSlotInfo* QuickSlotBarSlotInfo);
private:
	// Äü½½·Ô¹Ù 
	map<int8, CQuickSlotBar*> _QuickSlotBars;
};

