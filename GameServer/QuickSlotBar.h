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
		
	void UpdateQuickSlotBarSlot(st_QuickSlotBarSlotInfo& QuickSlotBarSlotInfo);		

	st_QuickSlotBarSlotInfo* FindQuickSlot(int8 QuickSlotbarSlotIndex);	

	void QuickSlotBarEmpty();

	map<int8, st_QuickSlotBarSlotInfo*> GetQuickSlotBarInfo();
private:
	map<int8, st_QuickSlotBarSlotInfo*> _QuickSlotBarSlotInfos;
};

