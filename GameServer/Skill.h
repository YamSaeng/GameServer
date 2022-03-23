#pragma once
#include"GameObjectInfo.h"

class CGameObject;

class CSkill
{
public:	
	CSkill();	
	~CSkill();

	// 스킬 정보 반환
	st_SkillInfo* GetSkillInfo();

	// 스킬을 소유하고 있는 객체를 설정
	void SetOwner(CGameObject* Owner);
	// 스킬 정보 셋팅
	void SetSkillInfo(en_SkillCategory SkillCategory, st_SkillInfo* SkillInfo);

	// 쿨타임 시작
	void CoolTimeStart();
	// 상태이상 지속 시간 시작
	void StatusAbnormalDurationTimeStart();

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
};

