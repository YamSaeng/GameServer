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
	map<en_MoveDir, string> SkillAnimations; // ��ų �ִϸ��̼�
	en_SkillType NextComboSkill;
	string SkillExplanation;
	string SkillThumbnailImagePath;
};

struct st_AttackSkillData : public st_SkillData
{
	int32 SkillMinDamage;		// �ּ� ���ݷ�
	int32 SkillMaxDamage;		// �ִ� ���ݷ�
	int8 SkillDebufAttackSpeed; // ��ų ���ݼӵ� ���� ��ġ
	int8 SkillDebufMovingSpeed; // ��ų �̵��ӵ� ���� ��ġ
	int8 StatusAbnormalityProbability; // ���� �̻� ���� Ȯ��
};

struct st_TacTicSkillData : public st_SkillData
{

};

struct st_HealSkillData : public st_TacTicSkillData
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
	string MonsterName;  // ���� �̸�
	st_ObjectStatusData MonsterStatInfo; // ���� ���� ����
	int32 SearchTick; // Ž�� �ӵ�
	int32 PatrolTick; // ���� �ӵ�
	int32 AttackTick; // ���� �ӵ�
	vector<st_DropData> DropItems; // ���Ͱ� ����ϴ� ������ ����
	int64 ReSpawnTime;
	int16 GetDPPoint;
	int32 GetExpPoint;
};

struct st_MonsterAggroData
{
	float MonsterAggroFirstTarget; // ���� ���� ��ǥ ��׷� ��
	float MonsterAggroFirstAttacker; // ���� ������ ��׷� ��
	float MonsterAggroAttacker; // ������ ��׷� ��
	float MonsterAggroHeal; // �� ��׷� ��
	float MonsterAggroGroupHeal; // �׷� �� ��׷� ��
	float MonsterAggroBuf; // ��ȭȿ�� ��׷� ��
	float MonsterAggroDebuf; //	��ȭȿ�� ��׷� ��	
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
	int64 MapID;			// �� ���� ID
	string MapName;			// �� �̸�
	int32 MapSectorSize;	// �� SectorSize
	int8 ChannelCount;		// ä�� ����
};