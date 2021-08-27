#pragma once
#include "Item.h"

struct st_ItemData
{
	int32 _DataSheetId;
	string _Name;
	en_ItemType _ItemType;
	string _ImagePath;
	bool IsEquipped;
	int32 Count;
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

struct st_StatusData
{
	int32 Level;
	int32 HP;
	int32 MaxHP;
	int32 Attack;
	int32 CriticalPoint;
	float Speed;
	int32 SearchCellDistance;
	int32 ChaseCellDistance;
	int32 AttackRange;
};

struct st_DropData
{
	int32 Probability;
	int32 ItemDataSheetId;
	int32 MinCount;
	int32 MaxCount;
};

struct st_MonsterData
{	
	int32 _MonsterDataId; // 몬스터 번호
	string _MonsterName;  // 몬스터 이름
	st_StatusData _MonsterStatInfo; // 몬스터 스탯 정보	
	vector<st_DropData> _DropItems; // 몬스터가 드랍하는 아이템 정보
};