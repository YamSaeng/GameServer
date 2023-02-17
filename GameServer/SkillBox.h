#pragma once
#include "GameObjectInfo.h"
#include "SkillCharacteristic.h"

class CSkillBox
{
public:
	CSkillBox();
	~CSkillBox();

	void Init();	
	void SetOwner(CGameObject* Owner);

	CSkillCharacteristic* FindCharacteristic(int8 FindCharacteristicIndex, int8 FindCharacteristicType);
	void CreateChracteristic(int8 ChracteristicIndex, int8 CharacteristicType);
	
	void SkillLearn(bool IsSkillLearn, int8 ChracteristicIndex, int8 CharacteristicType);

	CSkill* FindSkill(en_SkillCharacteristic CharacteristicType, en_SkillType SkillType);

	void Update();

	void Empty();	

	vector<CSkill*> GetGlobalSkills(en_SkillType ExceptSkillType, en_SkillKinds SkillKind);

	CSkillCharacteristic* GetSkillCharacteristicPublic();
	CSkillCharacteristic* GetSkillCharacteristics();

	bool CheckCharacteristic(en_SkillCharacteristic SkillCharacteristic);

	void SkillProcess(CGameObject* SkillUser, CGameObject* SkillUserd, en_SkillCharacteristic SkillCharacteristic, en_SkillType SkillType);
private:
	enum en_SkillCharacteristicCount
	{
		SKILL_CHARACTERISTIC_MAX_COUNT = 3
	};

	CSkillCharacteristic _SkillCharacteristicPublic;	
	CSkillCharacteristic _SkillCharacteristics[SKILL_CHARACTERISTIC_MAX_COUNT];	

	CSkill* _GlobalCoolTimeSkill;
	CGameObject* _OwnerGameObject;
};