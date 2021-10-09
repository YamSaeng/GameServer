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
	int32 DataSheetId;
	string ItemName;
	en_ItemCategory ItemCategory;
	en_ItemType ItemType; 
	string ThumbnailImagePath;
	bool IsEquipped;
	int16 ItemCount;
};

struct st_WeaponData : public st_ItemData
{
public:	
	int32 _Damage;
};

struct st_ArmorData : public st_ItemData
{
	int32 _Defence;
};

struct st_ConsumableData : public st_ItemData
{
	int32 _MaxCount;
};

struct st_MaterialData : public st_ItemData
{
	int32 _MaxCount;
};

struct st_PlayerStatusData
{
	int16 PlayerType;
	int32 Level;
	int32 HP;
	int32 MaxHP;
	int32 MP;
	int32 MaxMP;
	int32 DP;
	int32 MaxDP;
	int32 MinAttackDamage;
	int32 MaxAttackDamage;
	int16 CriticalPoint;
	float Speed;
	int32 SearchCellDistance;
	int32 ChaseCellDistance;
	int32 AttackRange;
};

struct st_DropData
{
	int32 Probability;
	int32 ItemDataSheetId;
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
	int32 SkillDataId;
	string SkillName;
	int32 SkillCoolTime;
	float SkillCastingTime;
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
	en_ItemType MaterialDataId; // ����� Id
	string MaterialName; // ��� �� �̸�
	string MaterialThumbnailImagePath; // ��� �� �̹��� ���
	int16 MaterialCount; // ��� �� ����
};

struct st_CraftingCompleteItemData
{
	en_ItemType CraftingCompleteItemDataId; // �ϼ����� ItemDataId
	string CraftingCompleteName; // ������ �̸�			
	string CraftingCompleteThumbnailImagePath; // ������ �̹��� ���
	vector<st_CraftingMaterialItemData> CraftingMaterials; // ���
};

struct st_CraftingItemCategoryData
{
	int32 CraftingDataId; // ������ DataId
	en_ItemCategory CraftingType; // ������ ����
	string CraftingTypeName; // ������ ���� �̸�	
	vector<st_CraftingCompleteItemData> CraftingCompleteItems; // ������ ���ֿ� ���� ������ ���
};