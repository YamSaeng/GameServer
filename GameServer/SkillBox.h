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
	
	void SkillLearn(bool IsSkillLearn, en_SkillType LearnSkillType);

	CSkill* FindSkill(en_SkillCharacteristic CharacteristicType, en_SkillType SkillType);

	void Update();

	void Empty();	

	vector<CSkill*> GetGlobalSkills(en_SkillType ExceptSkillType, en_SkillKinds SkillKind);

	CSkillCharacteristic* GetSkillCharacteristicPublic();
	CSkillCharacteristic* GetSkillCharacteristics();

	bool CheckCharacteristic(en_SkillCharacteristic SkillCharacteristic);

	void SkillProcess(CGameObject* SkillUser, CGameObject* SkillUserd, 
		en_SkillCharacteristic SkillCharacteristic, en_SkillType SkillType,
		float AttackDirectionX, float AttackDirectionY);
		
	int32 CalculateDamage(en_SkillType SkillType,
		int32& Str, int32& Dex, int32& Int, int32& Luck,
		bool* InOutCritical,
		int32 TargetDefence,
		int32 MinDamage, int32 MaxDamage,
		int16 CriticalPoint);

private:
	CSkillCharacteristic _SkillCharacteristicPublic;	
	CSkillCharacteristic _SkillCharacteristic;	

	CSkill* _GlobalCoolTimeSkill;
	CGameObject* _OwnerGameObject;
};