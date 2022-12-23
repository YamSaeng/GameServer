#pragma once
#include "GameObjectInfo.h"

class CMonsterSkillBox
{
public:
	CMonsterSkillBox();
	~CMonsterSkillBox();

	void Init(en_GameObjectType MonsterObjctType);

	vector<CSkill*> GetMonsterSkills();
private:
	vector<CSkill*> _MonsterSkills;
};

