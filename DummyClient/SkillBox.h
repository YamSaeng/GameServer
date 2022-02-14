#pragma once
#include<vector>>
#include"GameObjectInfo.h"

class CSkill;

class CSkillBox
{
private:
	//------------------------------
	// 스킬박스가 소유중인 스킬 목록
	//------------------------------
	vector<st_SkillInfo*> _Skills;
public:

	CSkillBox();
	~CSkillBox();

	void Init();

	// 스킬 추가
	void AddSkill(st_SkillInfo* SkillInfo);
	// 스킬 얻기
	st_SkillInfo* Get(int8 SlotIndex);
	// 스킬 찾기
	st_SkillInfo* FindSkill(en_SkillType FindSkillType);
};

