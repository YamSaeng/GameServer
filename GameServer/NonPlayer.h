#pragma once
#include"Creature.h"

class CNonPlayer : public CCreature
{
public:
	CNonPlayer();
	~CNonPlayer();

	virtual void Update() override;
	virtual bool OnDamaged(CGameObject* Attacker, int32 Damage) override;
	
protected:
	en_NonPlayerType _NonPlayerType;
};