#pragma once
#include "Monster.h"

class CBear : public CMonster
{
public:
	CBear();
	~CBear();

	virtual void Init(st_Vector2Int SpawnPosition) override;

	virtual void OnDead(CGameObject* Killer) override;
protected:
	virtual void UpdateIdle() override;
	virtual void UpdatePatrol() override;
	virtual void UpdateMoving() override;
	virtual void UpdateAttack() override;
	virtual void UpdateDead() override;
	virtual void UpdateSpawnIdle() override;
};

