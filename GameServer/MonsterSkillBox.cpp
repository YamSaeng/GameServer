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
				CSkill* SlimeMonsterAttackSkill = G_ObjectManager->SkillCreate();
				st_AttackSkillInfo* SlimeAttackSkillInfo = (st_AttackSkillInfo*)G_ObjectManager->SkillInfoCreate(SlimeSkillIter.second->SkillType, 1);
				*SlimeAttackSkillInfo = *SlimeSkillIter.second;

				SlimeMonsterAttackSkill->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_ACTIVE_SKILL, SlimeAttackSkillInfo);

				_MonsterSkills.push_back(SlimeMonsterAttackSkill);
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

void CMonsterSkillBox::Update()
{
	for (CSkill* MonsterSkill : _MonsterSkills)
	{
		MonsterSkill->Update();
	}
}

vector<CSkill*> CMonsterSkillBox::GetMonsterSkills()
{
	return _MonsterSkills;
}
