#include "pch.h"
#include "SkillBox.h"

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

void CSkillBox::AddAttackSkill(st_SkillInfo* AttackSkillInfo)
{
	_AttackSkills.push_back(AttackSkillInfo);
}

void CSkillBox::AddTacTicSkill(st_SkillInfo* TacTicSkillInfo)
{
	_TacTicSkills.push_back(TacTicSkillInfo);
}

void CSkillBox::AddBufSkill(st_SkillInfo* BufSkillInfo)
{
	_BufSkills.push_back(BufSkillInfo);
}

st_SkillInfo* CSkillBox::FindSkill(en_SkillType SkillType)
{
	st_SkillInfo* FindSkillInfo = nullptr;

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
		FindSkillInfo = FindAttackSkill((en_SkillType)SkillType);
		break;
		// 도사 전술
	case en_SkillType::SKILL_TAIOIST_HEALING_LIGHT:
	case en_SkillType::SKILL_TAIOIST_HEALING_WIND:
		FindSkillInfo = FindTacTicSkill((en_SkillType)SkillType);
		break;
		// 전사 버프
	case en_SkillType::SKILL_KNIGHT_CHARGE_POSE:
		FindSkillInfo = FindBufSkill((en_SkillType)SkillType);
		break;
	}

	return FindSkillInfo;
}

st_SkillInfo* CSkillBox::FindAttackSkill(en_SkillType FindAttackSkillType)
{
	for (int SlotIndex = 0; SlotIndex < _AttackSkills.size(); SlotIndex++)
	{
		if (_AttackSkills[SlotIndex]->SkillType == FindAttackSkillType)
		{
			return _AttackSkills[SlotIndex];
		}
	}

	return nullptr;
}

st_SkillInfo* CSkillBox::FindTacTicSkill(en_SkillType FindTacTicSkillType)
{
	for (int SlotIndex = 0; SlotIndex < _TacTicSkills.size(); SlotIndex++)
	{
		if (_TacTicSkills[SlotIndex]->SkillType == FindTacTicSkillType)
		{
			return _TacTicSkills[SlotIndex];
		}
	}

	return nullptr;
}

st_SkillInfo* CSkillBox::FindBufSkill(en_SkillType FindBufSkillType)
{
	for (int SlotIndex = 0; SlotIndex < _BufSkills.size(); SlotIndex++)
	{
		if (_BufSkills[SlotIndex]->SkillType == FindBufSkillType)
		{
			return _BufSkills[SlotIndex];
		}
	}

	return nullptr;
}