#pragma once
#include"GameObjectInfo.h"

class CGameObject;

class CSkill
{
public:	
	// 연속기 스킬 활성 시간 틱
	int64 _ComboSkillTick;

	// 근접 공격 시간 틱
	int64 _MeleeAttackTick;

	int8 _QuickSlotBarIndex;
	int8 _QuickSlotBarSlotIndex;

	CSkill();	
	~CSkill();

	// 스킬 정보 반환
	st_SkillInfo* GetSkillInfo();

	// 스킬을 소유하고 있는 객체를 설정
	void SetOwner(CGameObject* Owner);
	// 스킬 정보 셋팅
	void SetSkillInfo(en_SkillCategory SkillCategory, st_SkillInfo* SkillInfo = nullptr, st_SkillInfo* PreviousSkillInfo = nullptr);

	// 쿨타임 시작
	void CoolTimeStart();
	// 상태이상 지속 시간 시작
	void StatusAbnormalDurationTimeStart();
	// 연속기 스킬 시작
	void ComboSkillStart(int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex, en_SkillType ComboSkilltype);
	// 근접 공격 스킬 시작
	void MeleeAttackSkillStart(int64 AttackEndTick);

	bool Update();	
private:				
	// 스킬 쿨타임 틱
	int64 _SkillCootimeTick;
	// 스킬 상태이상 적용 틱
	int64 _SkillDurationTick;
	// 스킬 도트 틱
	int64 _SkillDotTick;	

	// 스킬이 도트인지 여부
	bool _IsDot;	

	// 스킬 가지고 있는 대상
	CGameObject* _Owner;	

	en_SkillCategory _SkillCategory;
	st_SkillInfo* _SkillInfo;
	
	// 연속기 스킬에 사용하는 스킬 정보로
	// 연속기 스킬 활성화 이전 스킬의 정보를 담는다.
	st_SkillInfo* _PreviousSkillInfo;
	// 연속기 스킬타입
	en_SkillType _ComboSkillType;
};

