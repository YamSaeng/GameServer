#pragma once
#include "Monster.h"

class CSlime : public CMonster
{
public:
	CSlime();
	~CSlime();

	virtual void Init(st_Vector2Int SpawnPosition) override;	

	virtual void OnDead(CGameObject* Killer) override;

	virtual void PositionReset() override;
protected:
	virtual void UpdateIdle() override;
	virtual void UpdatePatrol() override;
	virtual void UpdateMoving() override;
	virtual void UpdateAttack() override;
	virtual void UpdateDead() override;
	virtual void UpdateSpawnIdle() override;
};

