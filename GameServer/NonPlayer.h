#pragma once
#include "GameObject.h"
#include "InventoryManager.h"

class CNonPlayer : public CGameObject
{
public:
	CInventoryManager _InventoryManager;

	CNonPlayer();
	~CNonPlayer();

	virtual void Update() override;
	virtual bool OnDamaged(CGameObject* Attacker, int32 Damage) override;

	void Init(en_NonPlayerType NonPlayerType);
private:
	en_NonPlayerType _NonPlayerType;
};

