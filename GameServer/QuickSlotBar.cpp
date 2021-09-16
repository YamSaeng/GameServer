#include "pch.h"
#include "QuickSlotBar.h"

void CQuickSlotBar::AddQuickSlotSkill(int8 QuickSlotBarSkillSlotIndex, st_SkillInfo* QuickSlotSkillInfo)
{
	_QuickSlotSkillInfos.insert(pair<int8,st_SkillInfo*>(QuickSlotBarSkillSlotIndex, QuickSlotSkillInfo));
}

void CQuickSlotBar::AddQuickSlotItem(int8 QuickSlotBarItemSlotIndex, st_ItemInfo* QuickSlotItemInfo)
{
	_QuickSlotItemInfos.insert(pair<int8, st_ItemInfo*>(QuickSlotBarItemSlotIndex, QuickSlotItemInfo));
}
