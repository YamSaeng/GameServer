#include "pch.h"
#include "SkillBox.h"
#include "Skill.h"
#include "ObjectManager.h"

CSkillBox::CSkillBox()
{
	_SkillCharacteristicPublic.SkillCharacteristicInit(en_SkillCharacteristic::SKILL_CATEGORY_PUBLIC);

	for (int8 i = 0; i < (int8)en_SkillCharacteristicCount::SKILL_CHARACTERISTIC_MAX_COUNT; i++)
	{
		_SkillCharacteristics[i]._SkillBoxIndex = i;
	}
}

CSkillBox::~CSkillBox()
{
	
}

void CSkillBox::Init()
{

}

CSkillCharacteristic* CSkillBox::FindCharacteristic(int8 FindCharacteristicIndex, int8 FindCharacteristicType)
{
	if (_SkillCharacteristics[FindCharacteristicIndex]._SkillCharacteristic == (en_SkillCharacteristic)FindCharacteristicType)
	{
		return &_SkillCharacteristics[FindCharacteristicIndex];
	}
	else
	{
		return nullptr;
	}	
}

void CSkillBox::CreateChracteristic(int8 ChracteristicIndex, int8 CharacteristicType)
{
	if (ChracteristicIndex > 2)
	{
		CRASH("Overflow")
	}

	_SkillCharacteristics[ChracteristicIndex].SkillCharacteristicInit((en_SkillCharacteristic)CharacteristicType);
}

CSkill* CSkillBox::FindSkill(en_SkillCharacteristic CharacteristicType, en_SkillType SkillType)
{
	CSkill* FindSkill = nullptr;

	switch (CharacteristicType)
	{	
	case en_SkillCharacteristic::SKILL_CATEGORY_PUBLIC:
		FindSkill = _SkillCharacteristicPublic.FindSkill(SkillType);
		break;
	case en_SkillCharacteristic::SKILL_CATEGORY_FIGHT:		
	case en_SkillCharacteristic::SKILL_CATEGORY_PROTECTION:		
	case en_SkillCharacteristic::SKILL_CATEGORY_SPELL:		
	case en_SkillCharacteristic::SKILL_CATEGORY_SHOOTING:		
	case en_SkillCharacteristic::SKILL_CATEGORY_DISCIPLINE:		
	case en_SkillCharacteristic::SKILL_CATEGORY_ASSASSINATION:
		{
			for (int8 i = 0; i < (int8)en_SkillCharacteristicCount::SKILL_CHARACTERISTIC_MAX_COUNT; i++)
			{
				FindSkill = _SkillCharacteristics[i].FindSkill(SkillType);
			}
		}
		break;	
	}	

	return FindSkill;
}

void CSkillBox::Update()
{
	for (int8 i = 0; i < (int8)en_SkillCharacteristicCount::SKILL_CHARACTERISTIC_MAX_COUNT; i++)
	{
		if (_SkillCharacteristics[i]._SkillCharacteristic != en_SkillCharacteristic::SKILL_CATEGORY_NONE)
		{
			_SkillCharacteristics[i].CharacteristicUpdate();
		}
	}	
}

void CSkillBox::Empty()
{
	for (int i = 0; i < (int)en_SkillCharacteristicCount::SKILL_CHARACTERISTIC_MAX_COUNT; i++)
	{
		_SkillCharacteristics[i].CharacteristicEmpty();
	}
}

vector<CSkill*> CSkillBox::GetGlobalSkills(en_SkillType ExceptSkillType, en_SkillKinds SkillKind)
{
	vector<CSkill*> GlobalSkills;

	// 기본 공격 스킬은 제외함

	// 요청한 스킬과 같은 종류의 스킬을 스킬특성 창에서 찾음
	for (int i = 0; i < (int)en_SkillCharacteristicCount::SKILL_CHARACTERISTIC_MAX_COUNT; i++)
	{
		vector<CSkill*> SkillCharacteristicActiveSkill = _SkillCharacteristics[i].GetActiveSkill();
		for (CSkill* ActiveSkill : SkillCharacteristicActiveSkill)
		{
			if (ActiveSkill->GetSkillInfo()->SkillType != ExceptSkillType
				&& ActiveSkill->GetSkillKind() == SkillKind
				&& ActiveSkill->GetSkillInfo()->CanSkillUse == true)
			{
				GlobalSkills.push_back(ActiveSkill);
			}
		}
	}	

	return GlobalSkills;
}