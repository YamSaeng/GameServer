#pragma once
#include "GameObject.h"

class CItem : public CGameObject
{
public:	
	st_ItemInfo _ItemInfo;	
	st_Vector2Int _SpawnPosition;

	int64 _OwnerObjectId;
	en_GameObjectType _OwnerObjectType;

	CItem();

	virtual void Update() override;
	void SetDestoryTime(int32 DestoryTime);
	void ItemSetTarget(en_GameObjectType TargetType, int64 TargetDBId);
protected:
	int64 _DestroyTime;

	virtual void UpdateIdle();
};

class CWeapon : public CItem
{

};

class CMaterial : public CItem
{
public:
	CMaterial();
};