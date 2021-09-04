#pragma once
#include "Item.h"

enum en_MonsterDataType
{
	SLIME_DATA = 1,
	BEAR_DATA
};

struct st_ItemData
{
	int32 DataSheetId;
	string ItemName;
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

struct st_StatusData
{
	int32 Level;
	int32 HP;
	int32 MaxHP;
	int32 Attack;
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
	int32 _MonsterDataId; // ���� ��ȣ
	string _MonsterName;  // ���� �̸�
	st_StatusData _MonsterStatInfo; // ���� ���� ����	
	vector<st_DropData> _DropItems; // ���Ͱ� ����ϴ� ������ ����
};