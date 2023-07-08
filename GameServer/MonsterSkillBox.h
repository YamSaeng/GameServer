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

	int32 CalculateDamage(int8 SkillKind,
		int32& Str, int32& Dex, int32& Int, int32& Luck,
		bool* InOutCritical,
		bool IsBackAttack,
		int32 TargetDefence,
		int32 MinDamage, int32 MaxDamage,
		int16 CriticalPoint);
private:
	CGameObject* _OwnerGameObject;

	CSkill* _MonsterGlobalCoolTimeSkill;
	vector<CSkill*> _MonsterSkills;
};

