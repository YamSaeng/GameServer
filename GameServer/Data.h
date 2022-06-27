#pragma once
#include "Item.h"

struct st_ItemData
{
	string ItemName;
	int32 ItemWidth;
	int32 ItemHeight;
	en_LargeItemCategory LargeItemCategory;
	en_MediumItemCategory MediumItemCategory;
	en_SmallItemCategory SmallItemCategory;
	en_GameObjectType ItemObjectType;	
	int32 ItemMaxHP;
	int32 ItemCraftingMaxHP;
	string ItemExplain;
	int32 ItemMinDamage;
	int32 ItemMaxDamage;
	int32 ItemDefence;
	int32 ItemMaxCount;
	string ItemThumbnailImagePath;
	bool ItemIsEquipped;
	int16 ItemCount;
	int64 ItemCraftingTime;
};

struct st_ConsumableData : public st_ItemData
{
	int16 HealPoint;
	en_SkillMediumCategory SkillMediumCategory;
	en_SkillType SkillType;
};

struct st_SkillData
{
	en_SkillLargeCategory SkillLargeCategory;
	en_SkillMediumCategory SkillMediumCategory;
	en_SkillType SkillType;
	string SkillName;
	int8 SkillLevel;
	int32 SkillCoolTime;
	int32 SkillCastingTime;
	int64 SkillDurationTime;
	int64 SkillDotTime;
	int SkillDistance;
	float SkillTargetEffectTime;
	map<en_MoveDir, string> SkillAnimations; // 스킬 애니메이션
	en_SkillType NextComboSkill;
	string SkillExplanation;
	string SkillThumbnailImagePath;
};

struct st_AttackSkillData : public st_SkillData
{
	int32 SkillMinDamage;		// 최소 공격력
	int32 SkillMaxDamage;		// 최대 공격력
	int8 SkillDebufAttackSpeed; // 스킬 공격속도 감소 수치
	int8 SkillDebufMovingSpeed; // 스킬 이동속도 감소 수치
	int8 StatusAbnormalityProbability; // 상태 이상 적용 확률
};

struct st_TacTicSkillData : public st_SkillData
{

};

struct st_HealSkillData : public st_TacTicSkillData
{
	int32 SkillMinHealPoint; // 최소 치유량
	int32 SkillMaxHealPoint; // 최대 치유량
};

struct st_BufSkillData : public st_SkillData
{
	int32 IncreaseMinAttackPoint; // 증가하는 최소 근접 공격력
	int32 IncreaseMaxAttackPoint; // 증가하는 최대 근접 공격력
	int32 IncreaseMeleeAttackSpeedPoint; // 증가하는 근접 공격 속도
	int16 IncreaseMeleeAttackHitRate; // 증가하는 근접 명중률	
	int16 IncreaseMagicAttackPoint; // 증가하는 마법 공격력
	int16 IncreaseMagicCastingPoint; // 증가하는 마법 캐스팅 속도
	int16 IncreaseMagicAttackHitRate; // 증가하는 마법 명중률		
	int32 IncreaseDefencePoint; // 증가하는 방어력 
	int16 IncreaseEvasionRate; // 증가하는 회피율
	int16 IncreaseMeleeCriticalPoint; // 증가하는 근접 치명타율
	int16 IncreaseMagicCriticalPoint; // 증가하는 마법 치명타율
	float IncreaseSpeedPoint; // 증가하는 이동 속도	
	int16 IncreaseStatusAbnormalityResistance; // 증가하는 상태이상저항값
};

struct st_GameObjectUIPositionData
{
	float NameBarPositionX;
	float NameBarPositionY;
	float HPBarPositionX;
	float HPBarPositionY;
	float SpellBarPositionX;
	float SpellBarPositionY;
};

struct st_ObjectStatusData
{
	en_GameObjectType PlayerType;
	int32 Level;
	int32 HP;
	int32 MaxHP;
	int32 MP;
	int32 MaxMP;
	int32 DP;
	int32 MaxDP;
	int16 AutoRecoveryHPPercent;
	int16 AutoRecoveryMPPercent;
	int32 MinMeleeAttackDamage;
	int32 MaxMeleeAttackDamage;
	int16 MeleeAttackHitRate;
	int16 MagicDamage;
	float MagicHitRate;
	int32 Defence;
	int16 EvasionRate;
	int16 MeleeCriticalPoint;
	int16 MagicCriticalPoint;
	float Speed;
	int32 SearchCellDistance;
	int32 ChaseCellDistance;
	float AttackRange;
};

struct st_LevelData
{
	int32 Level;
	int64 RequireExperience;
	int64 TotalExperience;
};

struct st_DropData
{
	int32 Probability;
	en_SmallItemCategory DropItemSmallCategory;
	int8 MinCount;
	int16 MaxCount;
};

struct st_MonsterData
{
	string MonsterName;  // 몬스터 이름
	st_ObjectStatusData MonsterStatInfo; // 몬스터 스탯 정보
	int32 SearchTick; // 탐색 속도
	int32 PatrolTick; // 정찰 속도
	int32 AttackTick; // 공격 속도
	vector<st_DropData> DropItems; // 몬스터가 드랍하는 아이템 정보
	int64 ReSpawnTime;
	int16 GetDPPoint;
	int32 GetExpPoint;
};

struct st_MonsterAggroData
{
	float MonsterAggroFirstTarget; // 최초 공격 목표 어그로 값
	float MonsterAggroFirstAttacker; // 최초 공격자 어그로 값
	float MonsterAggroAttacker; // 공격자 어그로 값
	float MonsterAggroHeal; // 힐 어그로 값
	float MonsterAggroGroupHeal; // 그룹 힐 어그로 값
	float MonsterAggroBuf; // 강화효과 어그로 값
	float MonsterAggroDebuf; //	약화효과 어그로 값	
};

struct st_EnvironmentData
{	
	string EnvironmentName;
	int32 Level;
	int32 MaxHP;
	int64 RecoveryTime;
	vector<st_DropData> DropItems;
};

struct st_CropData
{
	string CropName;
	int32 MaxHP;
	vector<st_DropData> DropItems;
};

struct st_MapInfoData
{
	int64 MapID;			// 맵 고유 ID
	string MapName;			// 맵 이름
	int32 MapSectorSize;	// 맵 SectorSize
	int8 ChannelCount;		// 채널 개수
};