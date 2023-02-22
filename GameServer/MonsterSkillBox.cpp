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
	_MonsterGlobalCoolTimeSkill = G_ObjectManager->SkillCreate();
	if (_MonsterGlobalCoolTimeSkill != nullptr)
	{
		_MonsterGlobalCoolTimeSkill->SetOwner(_OwnerGameObject);

		st_SkillInfo* GlobalCoolTimeSkillInfo = G_ObjectManager->SkillInfoCreate(en_SkillType::SKILL_GLOBAL_SKILL, 1);
		if (GlobalCoolTimeSkillInfo != nullptr)
		{
			_MonsterGlobalCoolTimeSkill->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_GLOBAL, GlobalCoolTimeSkillInfo);
		}
	}

	switch (MonsterObjctType)
	{		
	case en_GameObjectType::OBJECT_GOBLIN:
		{
			for (auto GoblinSkillIter : G_Datamanager->_GoblinSkillDatas)
			{
				CSkill* GoblinMonsterActiveSkill = G_ObjectManager->SkillCreate();
				st_SkillInfo* GoblinActiveSkillInfo = G_ObjectManager->SkillInfoCreate(GoblinSkillIter.second->SkillType, 1);
				*GoblinActiveSkillInfo = *GoblinSkillIter.second;

				GoblinMonsterActiveSkill->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_ACTIVE_SKILL, GoblinActiveSkillInfo);

				_MonsterSkills.push_back(GoblinMonsterActiveSkill);
			}
		}
		break;
	case en_GameObjectType::OBJECT_SLIME:
		{			
			for (auto SlimeSkillIter : G_Datamanager->_SlimeSkillDatas)
			{
				CSkill* SlimeMonsterAttackSkill = G_ObjectManager->SkillCreate();
				st_SkillInfo* SlimeAttackSkillInfo = G_ObjectManager->SkillInfoCreate(SlimeSkillIter.second->SkillType, 1);
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

void CMonsterSkillBox::SetOwner(CGameObject* Owner)
{
	_OwnerGameObject = Owner;
}

void CMonsterSkillBox::Update()
{
	_MonsterGlobalCoolTimeSkill->Update();

	for (CSkill* MonsterSkill : _MonsterSkills)
	{
		MonsterSkill->Update();
	}
}

vector<CSkill*> CMonsterSkillBox::GetMonsterSkills()
{
	return _MonsterSkills;
}

CSkill* CMonsterSkillBox::FindSkill(en_SkillType FindSkillType)
{
	for (CSkill* MonsterSkill : _MonsterSkills)
	{
		if (MonsterSkill->GetSkillInfo()->SkillType == FindSkillType)
		{
			return MonsterSkill;
		}
	}

	return nullptr;
}

void CMonsterSkillBox::SkillProcess(CGameObject* SkillMonster, CGameObject* SkillTargetObject, en_SkillType SkillType)
{
	CSkill* Skill = FindSkill(SkillType);
	if (Skill != nullptr)
	{
		if (_MonsterGlobalCoolTimeSkill->GetSkillInfo()->CanSkillUse == true)
		{
			if (Skill->GetSkillInfo()->CanSkillUse == true)
			{
				vector<st_FieldOfViewInfo> CurrentFieldOfViewInfos = SkillMonster->GetChannel()->GetMap()->GetFieldAroundPlayers(SkillMonster);

				CMessage* ResAnimationPlayPacket = G_ObjectManager->GameServer->MakePacketResAnimationPlay(SkillMonster->_GameObjectInfo.ObjectId,
					SkillMonster->_GameObjectInfo.ObjectType, en_AnimationType::ANIMATION_TYPE_SWORD_MELEE_ATTACK);
				G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewInfos, ResAnimationPlayPacket);
				ResAnimationPlayPacket->Free();

				switch (Skill->GetSkillInfo()->SkillType)
				{
				case en_SkillType::SKILL_GOBLIN_ACTIVE_MELEE_DEFAULT_ATTACK:
					{					
						CMonster* Monster = dynamic_cast<CMonster*>(SkillMonster);
						CGameObject* MonsterTarget = Monster->GetTarget();
						
						if (st_Vector2::CheckFieldOfView(MonsterTarget->_GameObjectInfo.ObjectPositionInfo.Position,
							SkillMonster->_GameObjectInfo.ObjectPositionInfo.Position,
							SkillMonster->_FieldOfDirection, SkillMonster->_FieldOfAngle, Skill->GetSkillInfo()->SkillDistance))
						{
							st_GameObjectJob* DamageJob = G_ObjectManager->GameServer->MakeGameObjectDamage(SkillMonster->_GameObjectInfo.ObjectId, SkillMonster->_GameObjectInfo.ObjectType,
								Skill->GetSkillInfo()->SkillType,
								Skill->GetSkillInfo()->SkillMinDamage,
								Skill->GetSkillInfo()->SkillMaxDamage);
							MonsterTarget->_GameObjectJobQue.Enqueue(DamageJob);
						}						
					}
					break;				
				}

				Skill->CoolTimeStart();

				_MonsterGlobalCoolTimeSkill->CoolTimeStart();
			}
		}
	}
}
