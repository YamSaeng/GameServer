#pragma once
#include"GameObjectInfo.h"

class CGameObject;

class CSkill
{
public:	
	CSkill();	
	~CSkill();

	// ��ų ���� ��ȯ
	st_SkillInfo* GetSkillInfo();

	// ��ų�� �����ϰ� �ִ� ��ü�� ����
	void SetOwner(CGameObject* Owner);
	// ��ų ���� ����
	void SetSkillInfo(en_SkillCategory SkillCategory, st_SkillInfo* SkillInfo);

	// ��Ÿ�� ����
	void CoolTimeStart();
	// �����̻� ���� �ð� ����
	void StatusAbnormalDurationTimeStart();

	bool Update();
private:		
	// ��ų ��Ÿ�� ƽ
	int64 _SkillCootimeTick;
	// ��ų �����̻� ���� ƽ
	int64 _SkillDurationTick;
	// ��ų ��Ʈ ƽ
	int64 _SkillDotTick;

	// ��ų�� ��Ʈ���� ����
	bool _IsDot;	

	// ��ų ������ �ִ� ���
	CGameObject* _Owner;	

	en_SkillCategory _SkillCategory;
	st_SkillInfo* _SkillInfo;
};

