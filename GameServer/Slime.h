#pragma once
#include "Monster.h"

class CSlime : public CMonster
{
public:
	CSlime();
	~CSlime();

	virtual void Init(st_Vector2Int SpawnPosition) override;		

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

