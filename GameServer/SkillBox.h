#pragma once
#include"GameObjectInfo.h"

class CSkill;

class CSkillBox
{
private:
	//------------------------------
	// ��ų�ڽ��� �������� ��ų ���
	//------------------------------
	map<byte, st_SkillInfo*> _Skills;
public:

	CSkillBox();
	~CSkillBox();

	void Init();
	
	// ��ų �߰�
	void AddSkill(st_SkillInfo SkillInfo);
	// ��ų ���
	st_SkillInfo* Get(int8 SlotIndex);
};
