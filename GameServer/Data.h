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

struct st_CraftingMaterialData
{
	en_ItemType MaterialDataId;
	int16 MaterialCount;
};

struct st_CraftingData
{
	int32 CraftingDataId;
	string CraftingName;
	en_ItemCategory CraftingType;
	en_ItemType CraftingCompleteItemDataId;
	vector<st_CraftingMaterialData> CraftingMaterials;
};