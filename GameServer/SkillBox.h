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
	void AddAttackSkill(CSkill* AttackSkillInfo);
	// ���� ��ų �߰�
	void AddTacTicSkill(CSkill* TacTicSkillInfo);
	// ��ȭ ȿ�� ��ų �߰�
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
	// ��ų�ڽ��� �������� ��ų ���
	//------------------------------
	vector<CSkill*> _AttackSkills;
	vector<CSkill*> _TacTicSkills;
	vector<CSkill*> _BufSkills;

	// ���� ��ų ã��
	CSkill* FindAttackSkill(en_SkillType FindAttackSkillType);
	// ���� ��ų ã��
	CSkill* FindTacTicSkill(en_SkillType FindTacTicSkillType);
	// ��ȭ ȿ�� ��ų ã��
	CSkill* FindBufSkill(en_SkillType FindBufSkillType);
};