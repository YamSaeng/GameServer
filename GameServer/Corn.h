#pragma once
#include "Crop.h"

class CCorn : public CCrop
{
public:
	CCorn();
	~CCorn();

	virtual bool OnDamaged(CGameObject* Attacker, int32 Damage) override; 
};

