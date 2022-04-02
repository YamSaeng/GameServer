#pragma once
#include "GameObjectInfo.h"

class CQuickSlotBar
{
public:
	map<int8, st_QuickSlotBarSlotInfo*> _QuickSlotBarSlotInfos;
	// Äü½½·Ô ¹Ù ÀÎµ¦½º
	int8 _QuickSlotBarIndex;

	CQuickSlotBar();
	~CQuickSlotBar();

	void Init();

	// QuickSlot¿¡ ½ºÅ³ ÀúÀå
	void UpdateQuickSlotBarSlot(st_QuickSlotBarSlotInfo& QuickSlotBarSlotInfo);	
	
	st_QuickSlotBarSlotInfo* FindQuickSlot(int8 QuickSlotbarSlotIndex);

	void QuickSlotBarEmpty();
};

