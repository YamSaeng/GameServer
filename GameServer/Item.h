#pragma once
#include "GameObject.h"

class CItem : public CGameObject
{	
public:			
	enum class en_ItemCrafting : int8
	{
		ITEM_CRAFTING_NONE,
		ITEM_CRAFTING_START,
		ITEM_CRAFTING_STOP
	};

	// ������ ���� ƽ
	int64 _CraftingTick;

	// ������ ����
	st_ItemInfo _ItemInfo;
	
	// ������ ������ ID
	int64 _OwnerObjectId;

	en_GameObjectType _OwnerObjectType;

	en_ItemCrafting _ItemCrafting;

	CItem();
	virtual ~CItem();

	virtual void Update() override;

	//-------------------------------------
	// ������ ���� ����
	//-------------------------------------
	void CraftingStart();
	//-------------------------------------
	// ������ ���� ����
	//-------------------------------------
	void CraftingStop();

	//-------------------------------------
	// �������� �ı��� �ð��� �����Ѵ�.
	//-------------------------------------
	void SetDestoryTime(int32 DestoryTime);
	
	//-----------------------------------------------------------------
	// �������� ���� �� �� �ִ� ����� �����Ѵ�.
	//-----------------------------------------------------------------
	void ItemSetTarget(en_GameObjectType TargetType, int64 TargetDBId);

	void Init();
protected:	
	// �ı��� �ð�	
	int64 _DestroyTime;

	virtual void UpdateIdle();
private:	
};

class CWeaponItem : public CItem
{
public:
	virtual void UpdateIdle() override;

	CWeaponItem();
	~CWeaponItem();
};

class CArmorItem : public CItem
{
public:
	virtual void UpdateIdle() override;

	CArmorItem();
	~CArmorItem();
};

class CConsumable : public CItem
{
	
};

class CMaterialItem : public CItem
{
public:
	virtual void UpdateIdle() override;

	CMaterialItem();
	~CMaterialItem();
};

class CArchitectureItem : public CItem
{
public:
	virtual void UpdateIdle() override;

	CArchitectureItem();
	~CArchitectureItem();
};

class CCropItem : public CItem
{
public:
	virtual void UpdateIdle() override;

	CCropItem();
	~CCropItem();
};