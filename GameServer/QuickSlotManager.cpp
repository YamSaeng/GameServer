#include "pch.h"
#include "QuickSlotManager.h"

void CQuickSlotManager::AddQuickSlotBar(int8 QuickSlotBarIndex, CQuickSlotBar* NewQuickSlotBar)
{
	_QuickSlotBars.insert(pair<int8, CQuickSlotBar*>(QuickSlotBarIndex, NewQuickSlotBar));
}
