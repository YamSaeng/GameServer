#pragma once
#include"GameObjectInfo.h"

class CSkill;

class CSkillBox
{
public:
	CSkillBox();
	~CSkillBox();

	void Init();

	// 공격 스킬 추가
	void AddAttackSkill(st_SkillInfo* AttackSkillInfo);	
	// 전술 스킬 추가
	void AddTacTicSkill(st_SkillInfo* TacTicSkillInfo);
	// 강화 효과 스킬 추가
	void AddBufSkill(st_SkillInfo* BufSkillInfo);

	st_SkillInfo* FindSkill(en_SkillType SkillType);
private:
	//------------------------------
	// 스킬박스가 소유중인 스킬 목록
	//------------------------------
	vector<st_SkillInfo*> _AttackSkills;
	vector<st_SkillInfo*> _TacTicSkills;
	vector<st_SkillInfo*> _BufSkills;

	// 공격 스킬 찾기
	st_SkillInfo* FindAttackSkill(en_SkillType FindAttackSkillType);
	// 전술 스킬 찾기
	st_SkillInfo* FindTacTicSkill(en_SkillType FindTacTicSkillType);
	// 강화 효과 스킬 찾기
	st_SkillInfo* FindBufSkill(en_SkillType FindBufSkillType);
};