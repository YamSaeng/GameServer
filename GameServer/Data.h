#pragma once
#include "Item.h"

enum en_ObjectDataType
{
	SLIME_DATA = 1,
	BEAR_DATA,
	STONE_DATA = 1,
	TREE_DATA
};

struct st_ItemData
{
	string ItemName;
	int32 ItemWidth;
	int32 ItemHeight;
	en_LargeItemCategory LargeItemCategory;
	en_MediumItemCategory MediumItemCategory;
	en_SmallItemCategory SmallItemCategory; 
	en_GameObjectType ItemObjectType;
	string ItemExplain;
	int32 ItemMinDamage;
	int32 ItemMaxDamage;
	int32 ItemDefence;
	int32 ItemMaxCount;
	string ItemThumbnailImagePath;
	bool ItemIsEquipped;
	int16 ItemCount;
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
	int SkillDistance;
	float SkillTargetEffectTime;
	string SkillThumbnailImagePath;
};

struct st_AttackSkillData : public st_SkillData
{
	int32 SkillMinDamage;		// 최소 공격력
	int32 SkillMaxDamage;		// 최대 공격력
	bool SkillDebuf;			// 스킬 디버프 여부
	int64 SkillDebufTime;	    // 스킬 디버프 시간
	int8 SkillDebufAttackSpeed; // 스킬 공격속도 감소 수치
	int8 SkillDebufMovingSpeed; // 스킬 이동속도 감소 수치
	bool SkillDebufStun;		// 스킬 스턴 여부
	bool SkillDebufPushAway;	// 스킬 밀려남 여부
	bool SkillDebufRoot;	    // 스킬 이동불가 여부	
	int64 SkillDamageOverTime;  // 스킬 도트 데미지 시간 간격	
	int8 StatusAbnormalityProbability; // 상태 이상 적용 확률
};

struct st_HealSkillData : public st_SkillData
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
	int16 MagicHitRate;
	int32 Defence;
	int16 EvasionRate;
	int16 MeleeCriticalPoint;
	int16 MagicCriticalPoint;
	float Speed;	
	int32 SearchCellDistance;
	int32 ChaseCellDistance;
	int32 AttackRange;
	
	// 각 레벨 마다 제공하는 스킬 데이터
	vector<st_SkillData> LevelSkills;
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
	int32 MonsterDataId; // 몬스터 번호
	string MonsterName;  // 몬스터 이름
	st_ObjectStatusData MonsterStatInfo; // 몬스터 스탯 정보
	int32 SearchTick; // 탐색 속도
	int32 PatrolTick; // 정찰 속도
	int32 AttackTick; // 공격 속도
	vector<st_DropData> DropItems; // 몬스터가 드랍하는 아이템 정보
	int16 GetDPPoint;
	int32 GetExpPoint;
};

struct st_EnvironmentData
{
	int32 EnvironmentDataId;
	string EnvironmentName;
	int32 Level;
	int32 MaxHP;
	vector<st_DropData> DropItems;
};

struct st_CraftingMaterialItemData
{
	en_SmallItemCategory MaterialDataId; // 재료템 Id
	string MaterialName; // 재료 템 이름
	string MaterialThumbnailImagePath; // 재료 템 이미지 경로
	int16 MaterialCount; // 재료 템 개수
};

struct st_CraftingCompleteItemData
{
	en_SmallItemCategory CraftingCompleteItemDataId; // 완성템의 ItemDataId
	string CraftingCompleteName; // 제작템 이름			
	string CraftingCompleteThumbnailImagePath; // 제작템 이미지 경로
	vector<st_CraftingMaterialItemData> CraftingMaterials; // 재료
};

struct st_CraftingItemCategoryData
{	
	en_LargeItemCategory CraftingType; // 제작템 범주
	string CraftingTypeName; // 제작템 범주 이름	
	vector<st_CraftingCompleteItemData> CraftingCompleteItems; // 제작템 범주에 속한 아이템 목록
};