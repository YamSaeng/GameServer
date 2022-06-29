#pragma once
#include "Crop.h"

class CPotato : public CCrop
{
public:
	CPotato();
	~CPotato();

	virtual bool OnDamaged(CGameObject* Attacker, int32 Damage) override;
};

