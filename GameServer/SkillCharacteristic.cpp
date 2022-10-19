#include "pch.h"
#include "SkillCharacteristic.h"
#include "ObjectManager.h"
#include "DataManager.h"
#include "Skill.h"

CSkillCharacteristic::CSkillCharacteristic()
{
	_SkillBoxIndex = 0;
	_SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_NONE;
}

CSkillCharacteristic::~CSkillCharacteristic()
{
	CharacteristicEmpty();
}

void CSkillCharacteristic::SkillCharacteristicInit(en_SkillCharacteristic SkillCharacteristic)
{
	switch (SkillCharacteristic)
	{	
	case en_SkillCharacteristic::SKILL_CATEGORY_NONE:
		break;	
	case en_SkillCharacteristic::SKILL_CATEGORY_FIGHT:
		{
			int16 StartFightPassiveSkillType = (int16)en_SkillType::SKILL_FIGHT_TWO_HAND_SWORD_MASTER;
			int16 EndFightPassiveSkillType = (int16)en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FIERCE_ATTACK;

			// 격투 패시브 스킬 생성
			for (int16 i = StartFightPassiveSkillType; i < EndFightPassiveSkillType; i++)
			{
				CSkill* PassiveSkill = G_ObjectManager->SkillCreate();

				st_SkillInfo* SkillInfo = G_Datamanager->FindSkillData((en_SkillType)i);
				PassiveSkill->SetSkillInfo(en_SkillCategory::PASSIVE_SKILL, SkillInfo);

				_PassiveSkills.push_back(PassiveSkill);
			}

			int16 StartFightActiveSkillType = (int16)en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FIERCE_ATTACK;
			int16 EndFightActiveSkilltype = (int16)en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_SMASH;
	
			// 격투 액티브 스킬 생성
			for (int16 i = StartFightActiveSkillType; i < EndFightActiveSkilltype; i++)
			{
				CSkill* ActiveSkill = G_ObjectManager->SkillCreate();

				st_SkillInfo* ActiveSkillInfo = G_Datamanager->FindSkillData((en_SkillType)i);
				ActiveSkill->SetSkillInfo(en_SkillCategory::QUICK_SLOT_SKILL_COOLTIME, ActiveSkillInfo);

				_ActiveSkills.push_back(ActiveSkill);
			}
		}
		break;
	case en_SkillCharacteristic::SKILL_CATEGORY_PROTECTION:
		{
			int16 StartFightPassiveSkillType = (int16)en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_SMASH;
			int16 EndFightPassiveSkillType = (int16)en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_FLAME_HARPOON;

			for (int16 i = StartFightPassiveSkillType; i < EndFightPassiveSkillType; i++)
			{
				CSkill* ActiveSkill = G_ObjectManager->SkillCreate();

				st_SkillInfo* ActiveSkillInfo = G_Datamanager->FindSkillData((en_SkillType)i);
				ActiveSkill->SetSkillInfo(en_SkillCategory::QUICK_SLOT_SKILL_COOLTIME, ActiveSkillInfo);

				_ActiveSkills.push_back(ActiveSkill);
			}
		}
		break;
	case en_SkillCharacteristic::SKILL_CATEGORY_SPELL:
		{
			int16 StartFightPassiveSkillType = (int16)en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_FLAME_HARPOON;
			int16 EndFightPassiveSkillType = (int16)en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_DIVINE_STRIKE;

			for (int16 i = StartFightPassiveSkillType; i < EndFightPassiveSkillType; i++)
			{
				CSkill* ActiveSkill = G_ObjectManager->SkillCreate();

				st_SkillInfo* ActiveSkillInfo = G_Datamanager->FindSkillData((en_SkillType)i);
				ActiveSkill->SetSkillInfo(en_SkillCategory::QUICK_SLOT_SKILL_COOLTIME, ActiveSkillInfo);

				_ActiveSkills.push_back(ActiveSkill);
			}
		}
		break;
	case en_SkillCharacteristic::SKILL_CATEGORY_SHOOTING:
		{
			int16 StartFightPassiveSkillType = (int16)en_SkillType::SKILL_SHOOTING_ACTIVE_ATTACK_SNIFING;
			int16 EndFightPassiveSkillType = (int16)en_SkillType::SKILL_SLIME_NORMAL;

			for (int16 i = StartFightPassiveSkillType; i < EndFightPassiveSkillType; i++)
			{
				CSkill* ActiveSkill = G_ObjectManager->SkillCreate();

				st_SkillInfo* ActiveSkillInfo = G_Datamanager->FindSkillData((en_SkillType)i);
				ActiveSkill->SetSkillInfo(en_SkillCategory::QUICK_SLOT_SKILL_COOLTIME, ActiveSkillInfo);

				_ActiveSkills.push_back(ActiveSkill);
			}
		}
		break;
	case en_SkillCharacteristic::SKILL_CATEGORY_DISCIPLINE:
		{
			int16 StartFightPassiveSkillType = (int16)en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_DIVINE_STRIKE;
			int16 EndFightPassiveSkillType = (int16)en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_QUICK_CUT;

			for (int16 i = StartFightPassiveSkillType; i < EndFightPassiveSkillType; i++)
			{
				CSkill* ActiveSkill = G_ObjectManager->SkillCreate();

				st_SkillInfo* ActiveSkillInfo = G_Datamanager->FindSkillData((en_SkillType)i);
				ActiveSkill->SetSkillInfo(en_SkillCategory::QUICK_SLOT_SKILL_COOLTIME, ActiveSkillInfo);

				_ActiveSkills.push_back(ActiveSkill);
			}
		}
		break;
	case en_SkillCharacteristic::SKILL_CATEGORY_ASSASSINATION:
		{
			int16 StartFightPassiveSkillType = (int16)en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_QUICK_CUT;
			int16 EndFightPassiveSkillType = (int16)en_SkillType::SKILL_SHOOTING_ACTIVE_ATTACK_SNIFING;
	
			for (int16 i = StartFightPassiveSkillType; i < EndFightPassiveSkillType; i++)
			{
				CSkill* ActiveSkill = G_ObjectManager->SkillCreate();
	
				st_SkillInfo* ActiveSkillInfo = G_Datamanager->FindSkillData((en_SkillType)i);
				ActiveSkill->SetSkillInfo(en_SkillCategory::QUICK_SLOT_SKILL_COOLTIME, ActiveSkillInfo);
	
				_ActiveSkills.push_back(ActiveSkill);
			}
		}
		break;
	}	
}

void CSkillCharacteristic::SkillCharacteristicActive(en_SkillType SkillType, int8 SkillLevel)
{
	CSkill* ActiveSkill = FindSkill(SkillType);
	if (ActiveSkill != nullptr)
	{
		ActiveSkill->GetSkillInfo()->IsSkillLearn = true;
	}
}

CSkill* CSkillCharacteristic::FindSkill(en_SkillType FindSkillType)
{
	CSkill* FindPassiveSkill = FindPassiveSkills(FindSkillType);
	if (FindPassiveSkill == nullptr)
	{
		return FindActiveSkills(FindSkillType);
	}	
	
	return FindPassiveSkill;
}

CSkill* CSkillCharacteristic::FindPassiveSkills(en_SkillType FindPassiveSkillType)
{
	for (int SlotIndex = 0; SlotIndex < _PassiveSkills.size(); SlotIndex++)
	{
		if (_PassiveSkills[SlotIndex]->GetSkillInfo()->SkillType == FindPassiveSkillType)
		{
			return _PassiveSkills[SlotIndex];
		}
	}

	return nullptr;
}

CSkill* CSkillCharacteristic::FindActiveSkills(en_SkillType FindActiveSkillType)
{
	for (int SlotIndex = 0; SlotIndex < _ActiveSkills.size(); SlotIndex++)
	{
		if (_ActiveSkills[SlotIndex]->GetSkillInfo()->SkillType == FindActiveSkillType)
		{
			return _ActiveSkills[SlotIndex];
		}
	}

	return nullptr;
}

void CSkillCharacteristic::CharacteristicEmpty()
{
	for (CSkill* PassiveSkill : _PassiveSkills)
	{
		G_ObjectManager->SkillInfoReturn(PassiveSkill->GetSkillInfo()->SkillType, PassiveSkill->GetSkillInfo());
		G_ObjectManager->SkillReturn(PassiveSkill);
	}

	for (CSkill* ActiveSkill : _ActiveSkills)
	{
		G_ObjectManager->SkillInfoReturn(ActiveSkill->GetSkillInfo()->SkillType, ActiveSkill->GetSkillInfo());
		G_ObjectManager->SkillReturn(ActiveSkill);
	}

	_PassiveSkills.clear();
	_ActiveSkills.clear();
}

void CSkillCharacteristic::CharacteristicUpdate()
{
	for (CSkill* ActiveSkill : _ActiveSkills)
	{
		ActiveSkill->Update();
	}	
}

vector<CSkill*> CSkillCharacteristic::GetActiveSkill()
{
	return _ActiveSkills;
}
