#include "pch.h"
#include "QuickSlotBar.h"
#include "ObjectManager.h"

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
	for (int8 SlotIndex = 0; SlotIndex < (int8)en_QuickSlotBar::QUICK_SLOT_BAR_SLOT_SIZE; ++SlotIndex)
	{
		st_QuickSlotBarSlotInfo* QuickSlotBarSlotInfo = new st_QuickSlotBarSlotInfo();
		QuickSlotBarSlotInfo->QuickSlotBarIndex = _QuickSlotBarIndex;
		QuickSlotBarSlotInfo->QuickSlotBarSlotIndex = SlotIndex;
		QuickSlotBarSlotInfo->QuickSlotKey = SlotIndex + 1;		
		QuickSlotBarSlotInfo->QuickBarSkill = nullptr;

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

st_QuickSlotBarSlotInfo* CQuickSlotBar::FindQuickSlot(int8 QuickSlotbarSlotIndex)
{
	auto FindQuickSlotIterator = _QuickSlotBarSlotInfos.find(QuickSlotbarSlotIndex);
	if (FindQuickSlotIterator == _QuickSlotBarSlotInfos.end())
	{
		return nullptr;
	}

	return (*FindQuickSlotIterator).second;
}

void CQuickSlotBar::QuickSlotBarEmpty()
{
	for (auto QuickSlotBarSlot : _QuickSlotBarSlotInfos)
	{	
		delete QuickSlotBarSlot.second;
		QuickSlotBarSlot.second = nullptr;
	}

	_QuickSlotBarSlotInfos.clear();
}
