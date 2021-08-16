#pragma once
#include "Item.h"

struct st_ItemData
{
	int32 _DataSheetId;
	string _Name;
	en_ItemType _ItemType;
	string _ImagePath;
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

struct st_StatusData
{
	int32 Level;
	int32 HP;
	int32 MaxHP;
	int32 Attack;
	float Speed;
	int32 SearchCellDistance;
	int32 ChaseCellDistance;
	int32 AttackRange;
};

struct st_DropData
{
	int32 _Probability;
	int32 _ItemDataId;
	int32 _Count;
};

struct st_MonsterData
{	
	int32 _MonsterDataId; // ���� ��ȣ
	string _MonsterName;  // ���� �̸�
	st_StatusData _MonsterStatInfo; // ���� ���� ����	
	vector<st_DropData> _DropItems; // ���Ͱ� ����ϴ� ������ ����
};