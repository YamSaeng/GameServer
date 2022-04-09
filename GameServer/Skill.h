#pragma once
#include"GameObjectInfo.h"

class CGameObject;

class CSkill
{
public:	
	// ���ӱ� ��ų Ȱ�� �ð� ƽ
	int64 _ComboSkillTick;

	// ���� ���� �ð� ƽ
	int64 _MeleeAttackTick;

	int8 _QuickSlotBarIndex;
	int8 _QuickSlotBarSlotIndex;

	CSkill();	
	~CSkill();

	// ��ų ���� ��ȯ
	st_SkillInfo* GetSkillInfo();

	// ��ų�� �����ϰ� �ִ� ��ü�� ����
	void SetOwner(CGameObject* Owner);
	// ��ų ���� ����
	void SetSkillInfo(en_SkillCategory SkillCategory, st_SkillInfo* SkillInfo = nullptr, st_SkillInfo* PreviousSkillInfo = nullptr);

	// ��Ÿ�� ����
	void CoolTimeStart();
	// �����̻� ���� �ð� ����
	void StatusAbnormalDurationTimeStart();
	// ���ӱ� ��ų ����
	void ComboSkillStart(int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex, en_SkillType ComboSkilltype);
	// ���� ���� ��ų ����
	void MeleeAttackSkillStart(int64 AttackEndTick);

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
	
	// ���ӱ� ��ų�� ����ϴ� ��ų ������
	// ���ӱ� ��ų Ȱ��ȭ ���� ��ų�� ������ ��´�.
	st_SkillInfo* _PreviousSkillInfo;
	// ���ӱ� ��ųŸ��
	en_SkillType _ComboSkillType;
};

