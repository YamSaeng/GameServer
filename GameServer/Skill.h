#pragma once
#include"GameObjectInfo.h"

class CGameObject;

class CSkill
{
public:		
	// 스킬이 퀵슬롯바에 등록되어 있는 위치
	vector<st_Vector2Int> _QuickSlotBarPosition;	

	// 연속기 스킬이 활성화되어 있는 위치
	vector<st_Vector2Int> _ComboSkillQuickSlotBarIndex;	
	
	CSkill();	
	~CSkill();

	// 스킬 정보 반환
	st_SkillInfo* GetSkillInfo();

	// 스킬을 소유하고 있는 객체를 설정
	void SetOwner(CGameObject* Owner);
	// 스킬 정보 셋팅
	void SetSkillInfo(en_SkillCategory SkillCategory, st_SkillInfo* SkillInfo = nullptr, st_SkillInfo* PreviousSkillInfo = nullptr);
	
	void SetCastingUserID(int64 CastingUserID);
	int64 GetCastingUserID();


	// 쿨타임 시작
	void CoolTimeStart();
	// 전역 쿨타임 시작
	void GlobalCoolTimeStart(int32 GlobalCoolTime);

	// 상태이상 지속 시간 시작
	void StatusAbnormalDurationTimeStart();
	// 연속기 스킬 시작
	void ComboSkillStart(vector<st_Vector2Int> ComboSkillQuickSlotIndex, en_SkillType ComboSkilltype);
	// 물리 스킬 시작
	void ReqMeleeSkillInit(int64 AttackEndTick);
	// 마법 스킬 시작
	void ReqMagicSkillInit(float MagicHitRate);

	en_SkillKinds GetSkillKind();
	en_BufDeBufSkillKind GetBufDeBufSkillKind();

	void ComboSkillOff();

	bool Update();		
private:			
	// 스킬이 적용되고 있는 대상
	CGameObject* _Owner;	
	// 스킬을 시전한 대상 ID
	int64 _CastingUserID;
	// 스킬 정보
	st_SkillInfo* _SkillInfo;
	// 연속기 스킬에 사용하는 스킬 정보로
	// 연속기 스킬 활성화 이전 스킬의 정보를 담는다.
	st_SkillInfo* _PreviousSkillInfo;
	// 스킬 쿨타임 틱
	int64 _SkillCootimeTick;
	// 스킬 상태이상 적용 틱
	int64 _SkillDurationTick;
	// 스킬 도트 틱
	int64 _SkillDotTick;		
	// 연속기 스킬 활성 시간 틱
	int64 _ComboSkillTick;

	// 근접 공격 시간 틱
	int64 _MeleeAttackTick;
	// 마법 요청 시간 틱
	int64 _MagicTick;

	// 스킬이 도트인지 여부
	bool _IsDot;		
		
	// 스킬 분류 ( Active, Passive , StatusAbnormal, Combo ) 
	en_SkillCategory _SkillCategory;	
	// 스킬 속성
	en_SkillCharacteristic _SkillCharacteristic;
	// 스킬 분류 ( Melee, Magic, Range )
	en_SkillKinds _SkillKind;
	// 스킬 분류 ( Buf, DeBuf )
	en_BufDeBufSkillKind _BufDeBufSkillKind;	
	// 연속기 스킬타입
	en_SkillType _ComboSkillType;		
};

