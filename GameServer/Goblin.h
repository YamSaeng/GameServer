#pragma once
#include "Monster.h"

class CGoblin : public CMonster
{
public:
	CGoblin();
	~CGoblin();

	virtual void Start() override;
	virtual bool OnDamaged(CGameObject* Attacker, int32 Damage) override;

	virtual void PositionReset() override;
protected:
	virtual bool UpdateSpawnIdle() override;
	virtual void UpdateIdle() override;
	virtual void UpdatePatrol() override;
	virtual void UpdateMoving() override;
	virtual void UpdateAttack() override;
	virtual void UpdateReadyDead() override;
	virtual void UpdateDead() override;
};

