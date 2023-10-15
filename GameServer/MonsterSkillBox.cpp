#include "pch.h"
#include "MonsterSkillBox.h"
#include "DataManager.h"
#include "NetworkManager.h"
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
		_MonsterGlobalCoolTimeSkill->SetTarget(_OwnerGameObject);

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

				CMessage* ResAnimationPlayPacket = G_NetworkManager->GetGameServer()->MakePacketResAnimationPlay(SkillMonster->_GameObjectInfo.ObjectId,
					SkillMonster->_GameObjectInfo.ObjectType, en_AnimationType::ANIMATION_TYPE_SWORD_MELEE_ATTACK);
				G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewInfos, ResAnimationPlayPacket);
				ResAnimationPlayPacket->Free();				

				switch (Skill->GetSkillInfo()->SkillType)
				{
				case en_SkillType::SKILL_GOBLIN_ACTIVE_MELEE_DEFAULT_ATTACK:
					{					
						CMonster* Monster = dynamic_cast<CMonster*>(SkillMonster);
						CGameObject* MonsterTarget = Monster->GetTarget();
						
						if (Vector2::CheckFieldOfView(MonsterTarget->_GameObjectInfo.ObjectPositionInfo.Position,
							SkillMonster->_GameObjectInfo.ObjectPositionInfo.Position,
							SkillMonster->_FieldOfDirection, SkillMonster->_FieldOfAngle, Skill->GetSkillInfo()->SkillDistance))
						{
							st_GameObjectJob* DamageJob = G_NetworkManager->GetGameServer()->MakeGameObjectDamage(SkillMonster->_GameObjectInfo.ObjectId, 
								SkillMonster->_GameObjectInfo.ObjectType,
								Skill->GetSkillInfo()->SkillKind,
								Skill->GetSkillInfo()->SkillMinDamage,
								Skill->GetSkillInfo()->SkillMaxDamage, false);
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

int32 CMonsterSkillBox::CalculateDamage(int8 SkillKind, int32& Str, int32& Dex, int32& Int, int32& Luck, bool* InOutCritical, bool IsBackAttack, int32 TargetDefence, int32 MinDamage, int32 MaxDamage, int16 CriticalPoint)
{
	int32 ChoiceRandomDamage = Math::RandomNumberInt(MinDamage, MaxDamage);

	int32 CriticalDamage = 0;

	if (*InOutCritical == true)
	{
		// 크리티컬 판단
		float CriticalPointCheck = CriticalPoint / 1000.0f;		
		bool IsCritical = Math::IsSuccess(CriticalPointCheck);

		*InOutCritical = IsCritical;

		CriticalDamage = IsCritical ? ChoiceRandomDamage * 2 : ChoiceRandomDamage;
	}
	else
	{
		CriticalDamage = ChoiceRandomDamage;
	}

	int32 FinalDamage = 0;

	switch ((en_SkillKinds)SkillKind)
	{
	case en_SkillKinds::SKILL_KIND_MELEE_SKILL:
		FinalDamage = (int32)((CriticalDamage + Str / 2) * (1 - ((float)TargetDefence / (100.0f + (float)TargetDefence))));
		break;
	case en_SkillKinds::SKILL_KIND_SPELL_SKILL:
		FinalDamage = (int32)((CriticalDamage + Int / 2) * (1 - ((float)TargetDefence / (100.0f + (float)TargetDefence))));
		break;
	case en_SkillKinds::SKILL_KIND_RANGE_SKILL:
		break;
	}

	float DefenceRate = (float)pow(((float)(200 - 1)) / 20, 2) * 0.01f;
	FinalDamage = (int32)(CriticalDamage * DefenceRate);

	if (IsBackAttack == true)
	{
		FinalDamage *= 2.0f;
	}

	return FinalDamage;
}
