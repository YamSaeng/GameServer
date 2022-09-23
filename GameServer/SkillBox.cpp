#include "pch.h"
#include "SkillBox.h"
#include "Skill.h"
#include "ObjectManager.h"

CSkillBox::CSkillBox()
{
}

CSkillBox::~CSkillBox()
{
	for (auto AttackSkillInfoIterator : _AttackSkills)
	{
		delete AttackSkillInfoIterator;
	}	

	for (auto TacTicSkillInfoIterator : _TacTicSkills)
	{
		delete TacTicSkillInfoIterator;
	}

	for (auto BufSkillInfoIterator : _BufSkills)
	{
		delete BufSkillInfoIterator;
	}
}

void CSkillBox::Init()
{

}

void CSkillBox::AddAttackSkill(CSkill* AttackSkillInfo)
{
	_AttackSkills.push_back(AttackSkillInfo);
}

void CSkillBox::AddTacTicSkill(CSkill* TacTicSkillInfo)
{
	_TacTicSkills.push_back(TacTicSkillInfo);
}

void CSkillBox::AddBufSkill(CSkill* BufSkillInfo)
{
	_BufSkills.push_back(BufSkillInfo);
}

CSkill* CSkillBox::FindSkill(en_SkillType SkillType)
{
	CSkill* FindSkill = nullptr;

	// 스킬 찾기
	switch ((en_SkillType)SkillType)
	{
		// 기본 공격
	case en_SkillType::SKILL_DEFAULT_ATTACK:
		// 전사 공격
	case en_SkillType::SKILL_KNIGHT_FIERCE_ATTACK:
	case en_SkillType::SKILL_KNIGHT_CONVERSION_ATTACK:
	case en_SkillType::SKILL_KNIGHT_SMASH_WAVE:
	case en_SkillType::SKILL_KNIGHT_SHAEHONE:
	case en_SkillType::SKILL_KNIGHT_CHOHONE:
	case en_SkillType::SKILL_KNIGHT_SHIELD_SMASH:
		// 주술사 공격
	case en_SkillType::SKILL_SHAMAN_FLAME_HARPOON:
	case en_SkillType::SKILL_SHAMAN_ROOT:
	case en_SkillType::SKILL_SHAMAN_ICE_CHAIN:
	case en_SkillType::SKILL_SHAMAN_ICE_WAVE:
	case en_SkillType::SKILL_SHAMAN_LIGHTNING_STRIKE:
	case en_SkillType::SKILL_SHAMAN_HELL_FIRE:
		// 도사 공격
	case en_SkillType::SKILL_TAIOIST_DIVINE_STRIKE:
	case en_SkillType::SKILL_TAIOIST_ROOT:
		FindSkill = FindAttackSkill((en_SkillType)SkillType);
		break;
		// 주술사 전술
	case en_SkillType::SKILL_SHAMAN_BACK_TELEPORT:
		// 도사 전술
	case en_SkillType::SKILL_TAIOIST_HEALING_LIGHT:
	case en_SkillType::SKILL_TAIOIST_HEALING_WIND:
		FindSkill = FindTacTicSkill((en_SkillType)SkillType);
		break;
		// 전사 버프
	case en_SkillType::SKILL_KNIGHT_CHARGE_POSE:
	case en_SkillType::SKILL_SHOCK_RELEASE:
		FindSkill = FindBufSkill((en_SkillType)SkillType);
		break;
	}

	return FindSkill;
}

void CSkillBox::Update()
{
	for (CSkill* AttackSkill : _AttackSkills)
	{
		AttackSkill->Update();
	}

	for (CSkill* TacTicSkill : _TacTicSkills)
	{
		TacTicSkill->Update();
	}

	for (CSkill* BufSkill : _BufSkills)
	{
		BufSkill->Update();
	}
}

void CSkillBox::Empty()
{
	for (CSkill* AttackSkill : _AttackSkills)
	{
		G_ObjectManager->SkillInfoReturn(AttackSkill->GetSkillInfo()->SkillMediumCategory, AttackSkill->GetSkillInfo());
		G_ObjectManager->SkillReturn(AttackSkill);
	}

	for (CSkill* TacTicSkill : _TacTicSkills)
	{
		G_ObjectManager->SkillInfoReturn(TacTicSkill->GetSkillInfo()->SkillMediumCategory, TacTicSkill->GetSkillInfo());
		G_ObjectManager->SkillReturn(TacTicSkill);
	}

	for (CSkill* BufSkill : _BufSkills)
	{
		G_ObjectManager->SkillInfoReturn(BufSkill->GetSkillInfo()->SkillMediumCategory, BufSkill->GetSkillInfo());
		G_ObjectManager->SkillReturn(BufSkill);
	}

	_AttackSkills.clear();
	_TacTicSkills.clear();
	_BufSkills.clear();
}

vector<CSkill*> CSkillBox::GetAttackSkill()
{
	return _AttackSkills;
}

vector<CSkill*> CSkillBox::GetTacTicSkill()
{
	return _TacTicSkills;
}

vector<CSkill*> CSkillBox::GetBufSkill()
{
	return _BufSkills;
}

vector<CSkill*> CSkillBox::GetGlobalSkills(en_SkillType ExceptSkillType, en_SkillKinds SkillKind)
{
	vector<CSkill*> GlobalSkills;

	// 기본 공격 스킬은 제외함

	// 요청한 스킬과 같은 종류의 스킬을 가져옴
	for (CSkill* Skill : _AttackSkills)
	{
		if (Skill->GetSkillInfo()->SkillType != en_SkillType::SKILL_DEFAULT_ATTACK 
			&& Skill->GetSkillInfo()->SkillType != ExceptSkillType
			&& Skill->GetSkillKind() == SkillKind
			&& Skill->GetSkillInfo()->CanSkillUse == true)
		{
			GlobalSkills.push_back(Skill);
		}
	}

	for (CSkill* Skill : _TacTicSkills)
	{
		if (Skill->GetSkillInfo()->SkillType != ExceptSkillType
			&& Skill->GetSkillKind() == SkillKind
			&& Skill->GetSkillInfo()->CanSkillUse == true)
		{
			GlobalSkills.push_back(Skill);
		}
	}

	for (CSkill* Skill : _BufSkills)
	{
		if (Skill->GetSkillInfo()->SkillType != ExceptSkillType
			&& Skill->GetSkillKind() == SkillKind
			&& Skill->GetSkillInfo()->CanSkillUse == true)
		{
			GlobalSkills.push_back(Skill);
		}
	}

	return GlobalSkills;
}

CSkill* CSkillBox::FindAttackSkill(en_SkillType FindAttackSkillType)
{
	for (int SlotIndex = 0; SlotIndex < _AttackSkills.size(); SlotIndex++)
	{
		if (_AttackSkills[SlotIndex]->GetSkillInfo()->SkillType == FindAttackSkillType)
		{
			return _AttackSkills[SlotIndex];
		}
	}

	return nullptr;
}

CSkill* CSkillBox::FindTacTicSkill(en_SkillType FindTacTicSkillType)
{
	for (int SlotIndex = 0; SlotIndex < _TacTicSkills.size(); SlotIndex++)
	{
		if (_TacTicSkills[SlotIndex]->GetSkillInfo()->SkillType == FindTacTicSkillType)
		{
			return _TacTicSkills[SlotIndex];
		}
	}

	return nullptr;
}

CSkill* CSkillBox::FindBufSkill(en_SkillType FindBufSkillType)
{
	for (int SlotIndex = 0; SlotIndex < _BufSkills.size(); SlotIndex++)
	{
		if (_BufSkills[SlotIndex]->GetSkillInfo()->SkillType == FindBufSkillType)
		{
			return _BufSkills[SlotIndex];
		}
	}

	return nullptr;
}