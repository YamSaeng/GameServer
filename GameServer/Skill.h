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
	// ���� ��û �ð� ƽ
	int64 _MagicTick;

	// ��ų�� �����Թٿ� ��ϵǾ� �ִ� ��ġ
	vector<st_Vector2Int> _QuickSlotBarPosition;	

	// ���ӱ� ��ų�� Ȱ��ȭ�Ǿ� �ִ� ��ġ
	vector<st_Vector2Int> _ComboSkillQuickSlotBarIndex;	
	
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
	// ���� ��Ÿ�� ����
	void GlobalCoolTimeStart(int32 GlobalCoolTime);

	// �����̻� ���� �ð� ����
	void StatusAbnormalDurationTimeStart();
	// ���ӱ� ��ų ����
	void ComboSkillStart(vector<st_Vector2Int> ComboSkillQuickSlotIndex, en_SkillType ComboSkilltype);
	// ���� ��ų ����
	void ReqMeleeSkillInit(int64 AttackEndTick);
	// ���� ��ų ����
	void ReqMagicSkillInit(float MagicHitRate);


	en_SkillKinds GetSkillKind();

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
	en_SkillKinds _SkillKind;
	st_SkillInfo* _SkillInfo;
	
	// ���ӱ� ��ų�� ����ϴ� ��ų ������
	// ���ӱ� ��ų Ȱ��ȭ ���� ��ų�� ������ ��´�.
	st_SkillInfo* _PreviousSkillInfo;
	// ���ӱ� ��ųŸ��
	en_SkillType _ComboSkillType;
};

