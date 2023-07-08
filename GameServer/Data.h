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
	st_StatInfo MonsterStatInfo; // ���� ���� ����
	int32 SearchTick; // Ž�� �ӵ�
	int32 PatrolTick; // ���� �ӵ�
	int32 AttackTick; // ���� �ӵ�
	vector<en_SmallItemCategory> EquipmentItems; // �⺻ ���� ��� ���
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
	int16 MapID;
	string MapName;
	int32 MapSectorSize;
	int8 ChannelCount;
	int32 Left;
	int32 Right;
	int32 Up;
	int32 Down;
	map<en_GameObjectType, vector<Vector2Int>> GameObjectList;
};