#include "pch.h"
#include "MonsterSkillBox.h"
#include "DataManager.h"
#include "ObjectManager.h"
#include "Skill.h"

CMonsterSkillBox::CMonsterSkillBox()
{
}

CMonsterSkillBox::~CMonsterSkillBox()
{
}

void CMonsterSkillBox::Init(en_GameObjectType MonsterObjctType)
{
	switch (MonsterObjctType)
	{		
	case en_GameObjectType::OBJECT_SLIME:
		{			
			for (auto SlimeSkillIter : G_Datamanager->_SlimeAttackSkillDatas)
			{
				CSkill* NewMonsterSkill = G_ObjectManager->SkillCreate();

				st_AttackSkillInfo* NewMonsterSkillInfo = SlimeSkillIter.second;
				NewMonsterSkill->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_ACTIVE_SKILL, NewMonsterSkillInfo);

				_MonsterSkills.push_back(NewMonsterSkill);
			}			
		}
		break;
	case en_GameObjectType::OBJECT_BEAR:
		{
			
		}
		break;	
	default:
		CRASH("Monster ObjectType None");
		break;
	}
}

vector<CSkill*> CMonsterSkillBox::GetMonsterSkills()
{
	return _MonsterSkills;
}
