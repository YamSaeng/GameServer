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
	float MonsterAggroSecondTarget; // 최초 공격 목표 대상에게 갈 수 없을 때 두번째로 발견한 목표 어그로 값
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