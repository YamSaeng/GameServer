#include "pch.h"
#include "QuickSlotManager.h"
#include "Skill.h"
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

st_QuickSlotBarSlotInfo* CQuickSlotManager::FindQuickSlotBar(int8 QuickSlotBarIndex, int8 QuickSlotbarSlotIndex)
{
	auto FindQuickSlotIterator = _QuickSlotBars.find(QuickSlotBarIndex);
	if (FindQuickSlotIterator != _QuickSlotBars.end())
	{
		return (*FindQuickSlotIterator).second->FindQuickSlot(QuickSlotbarSlotIndex);
	}
}

vector<st_QuickSlotBarPosition> CQuickSlotManager::FindQuickSlotBar(en_SkillType FindSkillType)
{
	vector<st_QuickSlotBarPosition> QuickSlotSkillPositions;

	for (auto QuickSlotBarIterator : _QuickSlotBars)
	{
		for (auto QuickSlotBarSlotIterator : QuickSlotBarIterator.second->_QuickSlotBarSlotInfos)
		{
			st_QuickSlotBarSlotInfo* SearchingQuickSlotBarSlot = QuickSlotBarSlotIterator.second;
			if (SearchingQuickSlotBarSlot->QuickBarSkill != nullptr && SearchingQuickSlotBarSlot->QuickBarSkill->GetSkillInfo()->SkillType == FindSkillType)
			{
				st_QuickSlotBarPosition SearchingCompleteQuickSlotPosition;
				SearchingCompleteQuickSlotPosition.QuickSlotBarIndex = SearchingQuickSlotBarSlot->QuickSlotBarIndex;
				SearchingCompleteQuickSlotPosition.QuickSlotBarSlotIndex = SearchingQuickSlotBarSlot->QuickSlotBarSlotIndex;

				QuickSlotSkillPositions.push_back(SearchingCompleteQuickSlotPosition);
			}
		}
	}

	return QuickSlotSkillPositions;
}

vector<st_QuickSlotBarPosition> CQuickSlotManager::ExceptionFindQuickSlotBar(int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex)
{
	vector<st_QuickSlotBarPosition> QuickSlotSkillPositions;

	for (auto QuickSlotBarIterator : _QuickSlotBars)
	{
		for (auto QuickSlotBarSlotIterator : QuickSlotBarIterator.second->_QuickSlotBarSlotInfos)
		{
			st_QuickSlotBarSlotInfo* SearchingQuickSlotBarSlot = QuickSlotBarSlotIterator.second;
			if (SearchingQuickSlotBarSlot->QuickBarSkill != nullptr && SearchingQuickSlotBarSlot->QuickBarSkill->GetSkillInfo()->CanSkillUse == true
				&& ((SearchingQuickSlotBarSlot->QuickSlotBarIndex == QuickSlotBarIndex
					&& SearchingQuickSlotBarSlot->QuickSlotBarSlotIndex == QuickSlotBarSlotIndex) == false))
			{
				st_QuickSlotBarPosition SearchingCompleteQuickSlotPosition;
				SearchingCompleteQuickSlotPosition.QuickSlotBarIndex = SearchingQuickSlotBarSlot->QuickSlotBarIndex;
				SearchingCompleteQuickSlotPosition.QuickSlotBarSlotIndex = SearchingQuickSlotBarSlot->QuickSlotBarSlotIndex;

				QuickSlotSkillPositions.push_back(SearchingCompleteQuickSlotPosition);
			}
		}
	}

	return QuickSlotSkillPositions;
}

void CQuickSlotManager::Empty()
{
	for (auto QuickSlotBarIterator : _QuickSlotBars)
	{
		QuickSlotBarIterator.second->QuickSlotBarEmpty();
		delete QuickSlotBarIterator.second;
		QuickSlotBarIterator.second = nullptr;
	}	

	_QuickSlotBars.clear();
}