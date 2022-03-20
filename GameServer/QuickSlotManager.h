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
	void UpdateQuickSlotBar(st_QuickSlotBarSlotInfo& QuickSlotBarSlotInfo);

	void SwapQuickSlot(st_QuickSlotBarSlotInfo& SwapAQuickSlotInfo, st_QuickSlotBarSlotInfo& SwapBQuickSlotInfo);
	
	CSkill* FindQuickSlotBar(int8 QuickSlotBarIndex,int8 QuickSlotbarSlotIndex);
	
	void Empty();
private:
	// Äü½½·Ô¹Ù 
	map<int8, CQuickSlotBar*> _QuickSlotBars;
};