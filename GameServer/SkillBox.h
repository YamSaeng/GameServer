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
	void AddAttackSkill(CSkill* AttackSkillInfo);
	// 전술 스킬 추가
	void AddTacTicSkill(CSkill* TacTicSkillInfo);
	// 강화 효과 스킬 추가
	void AddBufSkill(CSkill* BufSkillInfo);

	CSkill* FindSkill(en_SkillType SkillType);

	void Update();

	void Empty();

	vector<CSkill*> GetAttackSkill();
	vector<CSkill*> GetTacTicSkill();
	vector<CSkill*> GetBufSkill();

	vector<CSkill*> GetGlobalSkills(en_SkillType ExceptSkillType, en_SkillKinds SkillKind);
private:
	//------------------------------
	// 스킬박스가 소유중인 스킬 목록
	//------------------------------
	vector<CSkill*> _AttackSkills;
	vector<CSkill*> _TacTicSkills;
	vector<CSkill*> _BufSkills;

	// 공격 스킬 찾기
	CSkill* FindAttackSkill(en_SkillType FindAttackSkillType);
	// 전술 스킬 찾기
	CSkill* FindTacTicSkill(en_SkillType FindTacTicSkillType);
	// 강화 효과 스킬 찾기
	CSkill* FindBufSkill(en_SkillType FindBufSkillType);
};