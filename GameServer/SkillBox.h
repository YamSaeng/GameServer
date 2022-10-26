#pragma once
#include "GameObjectInfo.h"
#include "SkillCharacteristic.h"
class CSkillBox
{
public:
	CSkillBox();
	~CSkillBox();

	void Init();	

	CSkillCharacteristic* FindCharacteristic(int8 FindCharacteristicIndex, int8 FindCharacteristicType);
	void CreateChracteristic(int8 ChracteristicIndex, int8 CharacteristicType);
	
	CSkill* FindSkill(en_SkillCharacteristic CharacteristicType, en_SkillType SkillType);

	void Update();

	void Empty();	

	vector<CSkill*> GetGlobalSkills(en_SkillType ExceptSkillType, en_SkillKinds SkillKind);

	CSkillCharacteristic* GetSkillCharacteristicPublic();
	CSkillCharacteristic* GetSkillCharacteristics();
private:
	enum en_SkillCharacteristicCount
	{
		SKILL_CHARACTERISTIC_MAX_COUNT = 3
	};

	CSkillCharacteristic _SkillCharacteristicPublic;	
	CSkillCharacteristic _SkillCharacteristics[SKILL_CHARACTERISTIC_MAX_COUNT];	
};