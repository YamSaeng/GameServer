#include "pch.h"
#include "QuickSlotBar.h"

CQuickSlotBar::CQuickSlotBar()
{
}

CQuickSlotBar::~CQuickSlotBar()
{
	for (auto QuickSlotBarSlot : _QuickSlotBarSlotInfos)
	{
		delete QuickSlotBarSlot.second;
	}
}

void CQuickSlotBar::Init()
{
	WCHAR QuickSlotBarKeyString[10] = { 0 };

	for (int8 SlotIndex = 0; SlotIndex < (int8)en_QuickSlotBar::QUICK_SLOT_BAR_SLOT_SIZE; ++SlotIndex)
	{
		st_QuickSlotBarSlotInfo* QuickSlotBarSlotInfo = new st_QuickSlotBarSlotInfo();
		QuickSlotBarSlotInfo->QuickSlotBarIndex = _QuickSlotBarIndex;
		QuickSlotBarSlotInfo->QuickSlotBarSlotIndex = SlotIndex;

		wsprintf(QuickSlotBarKeyString, L"%d", SlotIndex + 1);
		QuickSlotBarSlotInfo->QuickSlotKey = QuickSlotBarKeyString;

		_QuickSlotBarSlotInfos.insert(pair<int8, st_QuickSlotBarSlotInfo*>(SlotIndex, QuickSlotBarSlotInfo));
	}
}

void CQuickSlotBar::UpdateQuickSlotBarSlot(st_QuickSlotBarSlotInfo& QuickSlotBarSlotInfo)
{
	auto FindQuickSlotBarSlotIterator = _QuickSlotBarSlotInfos.find(QuickSlotBarSlotInfo.QuickSlotBarSlotIndex);
	if (FindQuickSlotBarSlotIterator == _QuickSlotBarSlotInfos.end())
	{
		CRASH("AddQuickSlotBarSlot 퀵슬롯아이템 찾지 못함");
		return;
	}

	*(*FindQuickSlotBarSlotIterator).second = QuickSlotBarSlotInfo;
}