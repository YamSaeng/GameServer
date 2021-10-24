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
	// �������� �ı��� �ð��� �����Ѵ�.
	//-------------------------------------
	void SetDestoryTime(int32 DestoryTime);
	
	//-----------------------------------------------------------------
	// �������� ���� �� �� �ִ� ����� �����Ѵ�.
	//-----------------------------------------------------------------
	void ItemSetTarget(en_GameObjectType TargetType, int64 TargetDBId);
protected:

	//------------------
	// �ı��� �ð�
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