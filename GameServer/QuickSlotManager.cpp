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
		QuickSlotBar->_QuickSlotBarIndex = SlotIndex;
		QuickSlotBar->Init();

		_QuickSlotBars.insert(pair<int8, CQuickSlotBar*>(SlotIndex, QuickSlotBar));
	}
}

void CQuickSlotManager::UpdateQuickSlotBar(st_QuickSlotBarSlotInfo& QuickSlotBarSlotInfo)
{
	auto FindQuickSlotBarIterator = _QuickSlotBars.find(QuickSlotBarSlotInfo.QuickSlotBarIndex);
	if (FindQuickSlotBarIterator == _QuickSlotBars.end())
	{
		CRASH("UpdateQuickSlotBarSlot Äü½½·Ô Ã£Áö ¸øÇÔ");
		return;
	}

	(*FindQuickSlotBarIterator).second->UpdateQuickSlotBarSlot(QuickSlotBarSlotInfo);
}

void CQuickSlotManager::SwapQuickSlot(st_QuickSlotBarSlotInfo& SwapAQuickSlotInfo, st_QuickSlotBarSlotInfo& SwapBQuickSlotInfo)
{
	auto FindSwapAItemIterator = _QuickSlotBars.find(SwapAQuickSlotInfo.QuickSlotBarIndex);
	if (FindSwapAItemIterator != _QuickSlotBars.end())
	{
		(*FindSwapAItemIterator).second->UpdateQuickSlotBarSlot(SwapAQuickSlotInfo);
	}

	auto FindSwapBItemIterator = _QuickSlotBars.find(SwapBQuickSlotInfo.QuickSlotBarIndex);
	if (FindSwapBItemIterator != _QuickSlotBars.end())
	{
		(*FindSwapBItemIterator).second->UpdateQuickSlotBarSlot(SwapBQuickSlotInfo);
	}
}
