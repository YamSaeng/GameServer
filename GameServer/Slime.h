#pragma once
#include "Monster.h"

class CSlime : public CMonster
{
private:
protected:
	virtual void UpdateIdle() override;
	virtual void UpdateMoving() override;
	virtual void UpdateAttack() override;
	virtual void UpdateDead() override;
	virtual void GetRandomDropItem(CGameObject* Killer) override;
public:
	CSlime();
	~CSlime();

	void Init(int32 DataSheetId);	

	virtual void OnDead(CGameObject* Killer) override;
};

