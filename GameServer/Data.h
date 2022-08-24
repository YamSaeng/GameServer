#pragma once
#include "Item.h"

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
	float MonsterAggroSecondTarget; // ���� ���� ��ǥ ��󿡰� �� �� ���� �� �ι�°�� �߰��� ��ǥ ��׷� ��
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