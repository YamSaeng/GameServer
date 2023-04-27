#pragma once
#include "GameObjectInfo.h"

class CSkillCharacteristic
{
public:
	CSkillCharacteristic();
	~CSkillCharacteristic();

	int8 _SkillBoxIndex;
	en_SkillCharacteristic _SkillCharacteristic;

	//----------------------------------------------------------------------
	// 스킬 특성 초기화 작업
	//----------------------------------------------------------------------
	void SkillCharacteristicInit(en_SkillCharacteristic SkillCharacteristic);	
		
	//----------------------------------------------------------
	// 스킬 특성 스킬 활성화
	//----------------------------------------------------------
	void SkillCharacteristicActive(bool IsSkillLearn, en_SkillType SkillType, int8 SkillLevel);	

	//-----------------------------------------------------------
	// 스킬 특성창에서 스킬 찾기
	//-----------------------------------------------------------
	CSkill* FindSkill(en_SkillType FindSkillType);
	CSkill* FindPassiveSkills(en_SkillType FindPassiveSkillType);
	CSkill* FindActiveSkills(en_SkillType FindActiveSkillType);

	//-------------------------
	// 스킬 특성창 비우기
	//-------------------------
	void CharacteristicEmpty();

	//----------------------------------
	// 스킬 특성창에서 배운 스킬 Update
	//----------------------------------
	void CharacteristicUpdate();

	// 스킬 반환
	//-------------------------------
	// 패시브 스킬 반환
	//-------------------------------
	vector<CSkill*> GetPassiveSkill();
	//-------------------------------
	// 액티브 스킬 반환
	//-------------------------------
	vector<CSkill*> GetActiveSkill();		
private:
	vector<CSkill*> _PassiveSkills;
	vector<CSkill*> _ActiveSkills;	
};