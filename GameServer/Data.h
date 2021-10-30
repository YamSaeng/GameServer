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
	en_SkillType SkillType;
};

struct st_PlayerStatusData
{
	en_GameObjectType PlayerType;
	int32 Level;
	int32 HP;
	int32 MaxHP;
	int32 MP;
	int32 MaxMP;
	int32 DP;
	int32 MaxDP;
	int32 MinAttackDamage;
	int32 MaxAttackDamage;
	int32 Defence;
	int16 CriticalPoint;
	float Speed;
	int64 RequireExperience;
	int64 TotalExperience;
	int32 SearchCellDistance;
	int32 ChaseCellDistance;
	int32 AttackRange;
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
	st_PlayerStatusData MonsterStatInfo; // 몬스터 스탯 정보
	int32 SearchTick; // 탐색 속도
	int32 PatrolTick; // 정찰 속도
	int32 AttackTick; // 공격 속도
	vector<st_DropData> DropItems; // 몬스터가 드랍하는 아이템 정보
	int16 GetDPPoint;
};

struct st_SkillData
{
	en_SkillLargeCategory SkillLargeCategory;
	en_SkillType SkillType;
	string SkillName;
	int32 SkillCoolTime;
	int32 SkillCastingTime;
	int SkillDistance;
	string SkillThumbnailImagePath;
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