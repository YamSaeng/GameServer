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

	CSkillCharacteristic* FindCharacteristic(int8 FindCharacteristicType);
	void CreateChracteristic(int8 CharacteristicType);
	
	void SkillLearn(bool IsSkillLearn, int8 CharacteristicType);

	CSkill* FindSkill(en_SkillCharacteristic CharacteristicType, en_SkillType SkillType);

	void Update();

	void Empty();	

	vector<CSkill*> GetGlobalSkills(en_SkillType ExceptSkillType, en_SkillKinds SkillKind);

	CSkillCharacteristic* GetSkillCharacteristicPublic();
	CSkillCharacteristic* GetSkillCharacteristics();

	bool CheckCharacteristic(en_SkillCharacteristic SkillCharacteristic);

	void SkillProcess(CGameObject* SkillUser, CGameObject* SkillUserd, en_SkillCharacteristic SkillCharacteristic, en_SkillType SkillType);
private:
	CSkillCharacteristic _SkillCharacteristicPublic;	
	CSkillCharacteristic _SkillCharacteristic;	

	CSkill* _GlobalCoolTimeSkill;
	CGameObject* _OwnerGameObject;
};