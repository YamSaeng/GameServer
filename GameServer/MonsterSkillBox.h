#pragma once
#include "GameObjectInfo.h"

class CMonsterSkillBox
{
public:
	CMonsterSkillBox();
	~CMonsterSkillBox();

	void Init(en_GameObjectType MonsterObjctType);
	void SetOwner(CGameObject* Owner);

	void Update();

	vector<CSkill*> GetMonsterSkills();

	CSkill* FindSkill(en_SkillType FindSkillType);

	void SkillProcess(CGameObject* SkillMonster, CGameObject* SkillTargetObject, en_SkillType SkillType);
private:
	CGameObject* _OwnerGameObject;

	CSkill* _MonsterGlobalCoolTimeSkill;
	vector<CSkill*> _MonsterSkills;
};

