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

	// 아이템 제작 틱
	int64 _CraftingTick;

	// 아이템 정보
	st_ItemInfo _ItemInfo;
	
	// 아이템 소유자 ID
	int64 _OwnerObjectId;

	en_GameObjectType _OwnerObjectType;

	en_ItemCrafting _ItemCrafting;

	CItem();
	virtual ~CItem();

	virtual void Update() override;

	//-------------------------------------
	// 아이템 제작 시작
	//-------------------------------------
	void CraftingStart();
	//-------------------------------------
	// 아이템 제작 멈춤
	//-------------------------------------
	void CraftingStop();

	//-------------------------------------
	// 아이템이 파괴될 시간을 지정한다.
	//-------------------------------------
	void SetDestoryTime(int32 DestoryTime);
	
	//-----------------------------------------------------------------
	// 아이템을 루팅 할 수 있는 대상을 지정한다.
	//-----------------------------------------------------------------
	void ItemSetTarget(en_GameObjectType TargetType, int64 TargetDBId);

	void Init();
protected:	
	// 파괴될 시간	
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