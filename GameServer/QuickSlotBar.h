#pragma once
#include "GameObjectInfo.h"

class CQuickSlotBar
{
public:
	// QuickSlot에 스킬 저장
	void AddQuickSlotSkill(int8 QuickSlotBarSkillSlotIndex, st_SkillInfo* QuickSlotSkillInfo);
	// QuickSlot에 아이템 저장
	void AddQuickSlotItem(int8 QuickSlotBarItemSlotIndex, st_ItemInfo* QuickSlotItemInfo);
private:
	// 퀵슬롯에 저장해둘 Skill, Item
	map<int8, st_SkillInfo*> _QuickSlotSkillInfos;
	map<int8, st_ItemInfo*> _QuickSlotItemInfos;
};

