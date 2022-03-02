#pragma once
#include"GameObjectInfo.h"

class CSkill;

class CSkillBox
{
public:
	CSkillBox();
	~CSkillBox();

	void Init();

	// ���� ��ų �߰�
	void AddAttackSkill(st_SkillInfo* AttackSkillInfo);	
	// ���� ��ų �߰�
	void AddTacTicSkill(st_SkillInfo* TacTicSkillInfo);
	// ��ȭ ȿ�� ��ų �߰�
	void AddBufSkill(st_SkillInfo* BufSkillInfo);

	st_SkillInfo* FindSkill(en_SkillType SkillType);
private:
	//------------------------------
	// ��ų�ڽ��� �������� ��ų ���
	//------------------------------
	vector<st_SkillInfo*> _AttackSkills;
	vector<st_SkillInfo*> _TacTicSkills;
	vector<st_SkillInfo*> _BufSkills;

	// ���� ��ų ã��
	st_SkillInfo* FindAttackSkill(en_SkillType FindAttackSkillType);
	// ���� ��ų ã��
	st_SkillInfo* FindTacTicSkill(en_SkillType FindTacTicSkillType);
	// ��ȭ ȿ�� ��ų ã��
	st_SkillInfo* FindBufSkill(en_SkillType FindBufSkillType);
};