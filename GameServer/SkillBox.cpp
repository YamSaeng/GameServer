#include "pch.h"
#include "SkillBox.h"

CSkillBox::CSkillBox()
{
}

CSkillBox::~CSkillBox()
{
	for (auto SkillInfoIterator : _Skills)
	{
		delete SkillInfoIterator;
	}
}

void CSkillBox::Init()
{

}

void CSkillBox::AddSkill(st_SkillInfo& SkillInfo)
{
	st_SkillInfo* NewSkillInfo = new st_SkillInfo();
	*NewSkillInfo = SkillInfo;

	_Skills.push_back(NewSkillInfo);
}

st_SkillInfo* CSkillBox::Get(int8 SlotIndex)
{
	return nullptr;
}

st_SkillInfo* CSkillBox::FindSkill(en_SkillType FindSkillType)
{
	for (int SlotIndex = 0; SlotIndex < _Skills.size(); SlotIndex++)
	{
		if (_Skills[SlotIndex]->_SkillType == FindSkillType)
		{
			return _Skills[SlotIndex];
		}
	}

	return nullptr;
}
