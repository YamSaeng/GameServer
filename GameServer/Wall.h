#pragma once
#include "Environment.h"

class CWall : public CEnvironment
{
public:
	CWall();

	virtual void Start() override;
	virtual bool OnDamaged(CGameObject* Attacker, int32 Damage) override;
protected:
	en_WallType _WallType;
private:
	virtual void UpdateIdle() override;
};
