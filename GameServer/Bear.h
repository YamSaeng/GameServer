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

	void Init(int32 DataSheetId);

	virtual void OnDead(CGameObject* Killer) override;
};

