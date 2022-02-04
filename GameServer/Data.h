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
	int32 SkillMinDamage;		// �ּ� ���ݷ�
	int32 SkillMaxDamage;		// �ִ� ���ݷ�
	bool SkillDebuf;			// ��ų ����� ����
	int64 SkillDebufTime;	    // ��ų ����� �ð�
	int8 SkillDebufAttackSpeed; // ��ų ���ݼӵ� ���� ��ġ
	int8 SkillDebufMovingSpeed; // ��ų �̵��ӵ� ���� ��ġ
	bool SkillDebufStun;		// ��ų ���� ����
	bool SkillDebufPushAway;	// ��ų �з��� ����
	bool SkillDebufRoot;	    // ��ų �̵��Ұ� ����	
	int64 SkillDamageOverTime;  // ��ų ��Ʈ ������ �ð� ����	
	int8 StatusAbnormalityProbability; // ���� �̻� ���� Ȯ��
};

struct st_HealSkillData : public st_SkillData
{
	int32 SkillMinHealPoint; // �ּ� ġ����
	int32 SkillMaxHealPoint; // �ִ� ġ����
};

struct st_BufSkillData : public st_SkillData
{
	int32 IncreaseMinAttackPoint; // �����ϴ� �ּ� ���� ���ݷ�
	int32 IncreaseMaxAttackPoint; // �����ϴ� �ִ� ���� ���ݷ�
	int32 IncreaseMeleeAttackSpeedPoint; // �����ϴ� ���� ���� �ӵ�
	int16 IncreaseMeleeAttackHitRate; // �����ϴ� ���� ���߷�	
	int16 IncreaseMagicAttackPoint; // �����ϴ� ���� ���ݷ�
	int16 IncreaseMagicCastingPoint; // �����ϴ� ���� ĳ���� �ӵ�
	int16 IncreaseMagicAttackHitRate; // �����ϴ� ���� ���߷�		
	int32 IncreaseDefencePoint; // �����ϴ� ���� 
	int16 IncreaseEvasionRate; // �����ϴ� ȸ����
	int16 IncreaseMeleeCriticalPoint; // �����ϴ� ���� ġ��Ÿ��
	int16 IncreaseMagicCriticalPoint; // �����ϴ� ���� ġ��Ÿ��
	float IncreaseSpeedPoint; // �����ϴ� �̵� �ӵ�	
	int16 IncreaseStatusAbnormalityResistance; // �����ϴ� �����̻����װ�
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
	
	// �� ���� ���� �����ϴ� ��ų ������
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
	int32 MonsterDataId; // ���� ��ȣ
	string MonsterName;  // ���� �̸�
	st_ObjectStatusData MonsterStatInfo; // ���� ���� ����
	int32 SearchTick; // Ž�� �ӵ�
	int32 PatrolTick; // ���� �ӵ�
	int32 AttackTick; // ���� �ӵ�
	vector<st_DropData> DropItems; // ���Ͱ� ����ϴ� ������ ����
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