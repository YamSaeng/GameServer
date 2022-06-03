#pragma once
#include "Monster.h"

class CBear : public CMonster
{
public:
	CBear();
	~CBear();

	virtual void Start() override;

	virtual void PositionReset() override;
protected:
	virtual bool UpdateSpawnIdle() override;
	virtual void UpdateIdle() override;
	virtual void UpdatePatrol() override;
	virtual void UpdateMoving() override;
	virtual void UpdateAttack() override;
	virtual void UpdateDead() override;	
};

