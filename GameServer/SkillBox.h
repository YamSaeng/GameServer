#pragma once
#include"GameObjectInfo.h"

class CSkill;

class CSkillBox
{
private:
	//------------------------------
	// 스킬박스가 소유중인 스킬 목록
	//------------------------------
	map<byte, st_SkillInfo*> _Skills;
public:

	CSkillBox();
	~CSkillBox();

	void Init();
	
	// 스킬 추가
	void AddSkill(st_SkillInfo SkillInfo);
	// 스킬 얻기
	st_SkillInfo* Get(int8 SlotIndex);
};

