#pragma once
#include "GameObjectInfo.h"

class CMonsterSkillBox
{
public:
	CMonsterSkillBox();
	~CMonsterSkillBox();

	void Init(en_GameObjectType MonsterObjctType);

	void Update();

	vector<CSkill*> GetMonsterSkills();
private:
	vector<CSkill*> _MonsterSkills;
};

