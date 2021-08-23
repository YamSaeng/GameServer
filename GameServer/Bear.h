#pragma once
#include "Monster.h"

class CBear : public CMonster
{
protected:
	virtual void UpdateIdle() override;
	virtual void UpdateMoving() override;
	virtual void UpdateAttack() override;
	virtual void UpdateDead() override;
public:
	CBear();
	~CBear();
};

