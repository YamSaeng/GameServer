#pragma once
#include"GameObjectInfo.h"

class CGameObject;

class CSkill
{
public:		
	// ��ų�� �����Թٿ� ��ϵǾ� �ִ� ��ġ
	vector<st_QuickSlotBarPosition> _QuickSlotBarPosition;

	// ���ӱ� ��ų�� Ȱ��ȭ�Ǿ� �ִ� ��ġ
	vector<st_QuickSlotBarPosition> _ComboSkillQuickSlotBarPosition;
	
	CSkill();	
	~CSkill();

	// ��ų ���� ��ȯ
	st_SkillInfo* GetSkillInfo();

	// ��ų�� �����ϰ� �ִ� ��ü�� ����
	void SetTarget(CGameObject* Target);
	// ��ų ���� ����
	void SetSkillInfo(en_SkillCategory SkillCategory, st_SkillInfo* SkillInfo = nullptr);
	
	void SetCastingUserID(int64 CastingUserID, en_GameObjectType CastingUserObjectType);
	int64 GetCastingUserID();

	// ��Ÿ�� ����
	void CoolTimeStart();
	// ���� ��Ÿ�� ����
	void GlobalCoolTimeStart(int32 GlobalCoolTime);

	// ��ȭȿ�� ����
	void BufTimeStart();
	// �����̻� ���� �ð� ����
	void StatusAbnormalDurationTimeStart();
	// ���ӱ� ��ų ����
	void ComboSkillStart(vector<st_QuickSlotBarPosition> ComboSkillQuickSlotIndex);
	// ���� ��ų ����
	void ReqMeleeSkillInit(int64 AttackEndTick);
	// ���� ��ų ����
	void ReqMagicSkillInit(float MagicHitRate);

	void ComboSkillOff();

	bool Update();		
private:			
	// ��ų�� ����ǰ� �ִ� ���
	CGameObject* _Target;	
	// ��ų�� ������ ��� ID
	int64 _CastingUserID;
	// ��ų�� ������ ����� ObjectType
	en_GameObjectType _CastingUserObjectType;
	// ��ų ����
	st_SkillInfo* _SkillInfo;		
	// ��ų ��Ÿ�� ƽ
	int64 _SkillCootimeTick;
	// ��ų �����̻� ���� ƽ
	int64 _SkillDurationTick;
	// ��ų ��Ʈ ƽ
	int64 _SkillDotTick;		
	// ���ӱ� ��ų Ȱ�� �ð� ƽ
	int64 _ComboSkillTick;

	// ���� ���� �ð� ƽ
	int64 _MeleeAttackTick;
	// ���� ��û �ð� ƽ
	int64 _MagicTick;

	// ��ų�� ��Ʈ���� ����
	bool _IsDot;		
		
	// ��ų �з� ( Active, Passive , StatusAbnormal, Combo ) 
	en_SkillCategory _SkillCategory;			
	// ���ӱ� ��ųŸ��
	en_SkillType _ComboSkillType;	
};

