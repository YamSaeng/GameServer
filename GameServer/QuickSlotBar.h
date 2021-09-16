#pragma once
#include "GameObjectInfo.h"

class CQuickSlotBar
{
public:
	// QuickSlot�� ��ų ����
	void AddQuickSlotSkill(int8 QuickSlotBarSkillSlotIndex, st_SkillInfo* QuickSlotSkillInfo);
	// QuickSlot�� ������ ����
	void AddQuickSlotItem(int8 QuickSlotBarItemSlotIndex, st_ItemInfo* QuickSlotItemInfo);
private:
	// �����Կ� �����ص� Skill, Item
	map<int8, st_SkillInfo*> _QuickSlotSkillInfos;
	map<int8, st_ItemInfo*> _QuickSlotItemInfos;
};

