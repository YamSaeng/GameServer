#include "pch.h"
#include "QuickSlotManager.h"
#include "QuickSlotBar.h"

CQuickSlotManager::CQuickSlotManager()
{
}

CQuickSlotManager::~CQuickSlotManager()
{
	for (auto QuickSlotBar : _QuickSlotBars)
	{		
		delete QuickSlotBar.second;
	}
}

void CQuickSlotManager::Init()
{
	// Äü½½·Ô¹Ù »ý¼º
	for (int8 SlotIndex = 0; SlotIndex < (int8)en_QuickSlotBar::QUICK_SLOT_BAR_SIZE; ++SlotIndex)
	{
		CQuickSlotBar* QuickSlotBar = new CQuickSlotBar();	
		QuickSlotBar->_Index = SlotIndex;

		_QuickSlotBars.insert(pair<int8, CQuickSlotBar*>(SlotIndex, QuickSlotBar));
	}
}

void CQuickSlotManager::AddQuickSlotBarSlot(st_QuickSlotBarSlotInfo* QuickSlotBarSlotInfo)
{
	auto FindQuickSlotBarIterator = _QuickSlotBars.find(QuickSlotBarSlotInfo->QuickSlotBarIndex);
	if (FindQuickSlotBarIterator == _QuickSlotBars.end())
	{
		CRASH("AddQuickSlotBar Äü½½·Ô Ã£Áö ¸øÇÔ");
		return;
	}

	(*FindQuickSlotBarIterator).second->AddQuickSlotBarSlot(QuickSlotBarSlotInfo);
}
