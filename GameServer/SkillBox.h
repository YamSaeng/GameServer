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

	// 상태이상 값 적용
	bool SetStatusAbnormal(CGameObject* SkillUser, CGameObject* Target, en_GameObjectStatusType StatusType, en_SkillType SkillType,int8 SkillLevel);
	
	// 선택한 대상에게 스킬을 사용 할 수 있는지 확인
	bool SelectTargetSkillUse(CGameObject* SkillUser, CSkill* Skill);	
	// 충돌체 생성해서 충돌하는 대상 반환
	vector<CGameObject*> CollisionSkillUse(CGameObject* SkillUser, CSkill* Skill, en_CollisionPosition CollisionPositionType, 
		Vector2 CollisionCreatePosition, Vector2 CollisionCreateDir);

	void ShockReleaseUse(CGameObject* User, CSkill* ShockReleaseSkill);
	void MoveStatusAbnormalRelease(CGameObject* User);

	void SkillProcess(CGameObject* SkillUser,
		en_SkillCharacteristic SkillCharacteristic, en_SkillType SkillType,
		float WeaponPositionX, float WeaponPositionY,
		float AttackDirectionX, float AttackDirectionY);
		
	int32 CalculateDamage(en_SkillType SkillType,
		int32& Str, int32& Dex, int32& Int, int32& Luck,
		bool* InOutCritical,
		bool IsBackAttack,
		int32 TargetDefence,
		int32 MinDamage, int32 MaxDamage,
		int16 CriticalPoint);

private:
	CSkillCharacteristic _SkillCharacteristicPublic;	
	CSkillCharacteristic _SkillCharacteristic;	

	CSkill* _GlobalCoolTimeSkill;
	CGameObject* _OwnerGameObject;
};