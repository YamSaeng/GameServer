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
	int32 MonsterDataId; // ���� ��ȣ
	string MonsterName;  // ���� �̸�
	st_PlayerStatusData MonsterStatInfo; // ���� ���� ����
	int32 SearchTick; // Ž�� �ӵ�
	int32 PatrolTick; // ���� �ӵ�
	int32 AttackTick; // ���� �ӵ�
	vector<st_DropData> DropItems; // ���Ͱ� ����ϴ� ������ ����
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
	en_SmallItemCategory MaterialDataId; // ����� Id
	string MaterialName; // ��� �� �̸�
	string MaterialThumbnailImagePath; // ��� �� �̹��� ���
	int16 MaterialCount; // ��� �� ����
};

struct st_CraftingCompleteItemData
{
	en_SmallItemCategory CraftingCompleteItemDataId; // �ϼ����� ItemDataId
	string CraftingCompleteName; // ������ �̸�			
	string CraftingCompleteThumbnailImagePath; // ������ �̹��� ���
	vector<st_CraftingMaterialItemData> CraftingMaterials; // ���
};

struct st_CraftingItemCategoryData
{	
	en_LargeItemCategory CraftingType; // ������ ����
	string CraftingTypeName; // ������ ���� �̸�	
	vector<st_CraftingCompleteItemData> CraftingCompleteItems; // ������ ���ֿ� ���� ������ ���
};