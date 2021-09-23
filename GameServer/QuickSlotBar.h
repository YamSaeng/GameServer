#pragma once
#include "GameObjectInfo.h"

class CQuickSlotBar
{
public:
	// Äü½½·Ô ¹Ù ÀÎµ¦½º
	int8 _QuickSlotBarIndex;

	CQuickSlotBar();
	~CQuickSlotBar();

	void Init();

	// QuickSlot¿¡ ½ºÅ³ ÀúÀå
	void UpdateQuickSlotBarSlot(st_QuickSlotBarSlotInfo& QuickSlotBarSlotInfo);

	bool CanQuickSlotBarSlotuse(st_QuickSlotBarSlotInfo& FindQuickSlotBarSlotInfo);
private:
	map<int8, st_QuickSlotBarSlotInfo*> _QuickSlotBarSlotInfos;	
};

