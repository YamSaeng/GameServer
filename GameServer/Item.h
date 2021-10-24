#pragma once
#include "GameObject.h"

class CItem : public CGameObject
{
public:			
	st_ItemInfo _ItemInfo;
	
	int64 _OwnerObjectId;
	en_GameObjectType _OwnerObjectType;

	CItem();
	virtual ~CItem();

	virtual void Update() override;
	
	//-------------------------------------
	// 아이템이 파괴될 시간을 지정한다.
	//-------------------------------------
	void SetDestoryTime(int32 DestoryTime);
	
	//-----------------------------------------------------------------
	// 아이템을 루팅 할 수 있는 대상을 지정한다.
	//-----------------------------------------------------------------
	void ItemSetTarget(en_GameObjectType TargetType, int64 TargetDBId);
protected:

	//------------------
	// 파괴될 시간
	//------------------
	int64 _DestroyTime;

	virtual void UpdateIdle();
private:

};

class CWeapon : public CItem
{
public:
	virtual void UpdateIdle() override;

	CWeapon();
	~CWeapon();
};

class CArmor : public CItem
{
public:
	virtual void UpdateIdle() override;

	CArmor();
	~CArmor();
};

class CConsumable : public CItem
{
	
};

class CMaterial : public CItem
{
public:
	virtual void UpdateIdle() override;

	CMaterial();
	~CMaterial();
};