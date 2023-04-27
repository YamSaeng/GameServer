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
	// ��ų Ư�� �ʱ�ȭ �۾�
	//----------------------------------------------------------------------
	void SkillCharacteristicInit(en_SkillCharacteristic SkillCharacteristic);	
		
	//----------------------------------------------------------
	// ��ų Ư�� ��ų Ȱ��ȭ
	//----------------------------------------------------------
	void SkillCharacteristicActive(bool IsSkillLearn, en_SkillType SkillType, int8 SkillLevel);	

	//-----------------------------------------------------------
	// ��ų Ư��â���� ��ų ã��
	//-----------------------------------------------------------
	CSkill* FindSkill(en_SkillType FindSkillType);
	CSkill* FindPassiveSkills(en_SkillType FindPassiveSkillType);
	CSkill* FindActiveSkills(en_SkillType FindActiveSkillType);

	//-------------------------
	// ��ų Ư��â ����
	//-------------------------
	void CharacteristicEmpty();

	//----------------------------------
	// ��ų Ư��â���� ��� ��ų Update
	//----------------------------------
	void CharacteristicUpdate();

	// ��ų ��ȯ
	//-------------------------------
	// �нú� ��ų ��ȯ
	//-------------------------------
	vector<CSkill*> GetPassiveSkill();
	//-------------------------------
	// ��Ƽ�� ��ų ��ȯ
	//-------------------------------
	vector<CSkill*> GetActiveSkill();		
private:
	vector<CSkill*> _PassiveSkills;
	vector<CSkill*> _ActiveSkills;	
};