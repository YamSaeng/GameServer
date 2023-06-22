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
	// �����Թ� ����
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
		CRASH("UpdateQuickSlotBarSlot ������ ã�� ����");
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

vector<st_QuickSlotBarSlotInfo*> CQuickSlotManager::FindQuickSlotBarInfo(en_SkillType FindSkillType)
{
	vector<st_QuickSlotBarSlotInfo*> QuickSlotBarInfos;
	for (auto QuickSlotBarIterator : _QuickSlotBars)
	{
		for (auto QuickSlotBarSlotIterator : QuickSlotBarIterator.second->GetQuickSlotBarInfo())
		{
			QuickSlotBarInfos.push_back(QuickSlotBarSlotIterator.second);
		}
	}

	return QuickSlotBarInfos;
}

vector<st_QuickSlotBarPosition> CQuickSlotManager::FindQuickSlotBar(en_SkillType FindSkillType)
{
	vector<st_QuickSlotBarPosition> QuickSlotSkillPositions;

	for (auto QuickSlotBarIterator : _QuickSlotBars)
	{
		for (auto QuickSlotBarSlotIterator : QuickSlotBarIterator.second->GetQuickSlotBarInfo())
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

vector<st_QuickSlotBarPosition> CQuickSlotManager::GlobalCoolTimeFindQuickSlotBar(int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex, en_SkillKinds SkillKind)
{
	vector<st_QuickSlotBarPosition> QuickSlotSkillPositions;

	for (auto QuickSlotBarIterator : _QuickSlotBars)
	{
		for (auto QuickSlotBarSlotIterator : QuickSlotBarIterator.second->GetQuickSlotBarInfo())
		{
			// ������ �ٿ���  �����Ѵ�.
			// ��ϵǾ� �ִ� �����Թ� �� ��� �� �� �ְ� (= ���� ���ð��� ���� ), SkillKind�� ������, �Ű������� ���� ��ġ ���� ������ �������� ã�´�.
			st_QuickSlotBarSlotInfo* SearchingQuickSlotBarSlot = QuickSlotBarSlotIterator.second;
			if (SearchingQuickSlotBarSlot->QuickBarSkill != nullptr && SearchingQuickSlotBarSlot->QuickBarSkill->GetSkillInfo()->CanSkillUse == true
				&& ((SearchingQuickSlotBarSlot->QuickSlotBarIndex == QuickSlotBarIndex
					&& SearchingQuickSlotBarSlot->QuickSlotBarSlotIndex == QuickSlotBarSlotIndex) == false)
				&& SearchingQuickSlotBarSlot->QuickBarSkill->GetSkillInfo()->SkillKind == SkillKind
				&& SearchingQuickSlotBarSlot->QuickBarSkill->GetSkillInfo()->SkillType != en_SkillType::SKILL_PUBLIC_ACTIVE_BUF_SHOCK_RELEASE)
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

map<int8, CQuickSlotBar*> CQuickSlotManager::GetQuickSlotBar()
{
	return _QuickSlotBars;
}
