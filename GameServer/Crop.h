#pragma once
#include "GameObject.h"

class CCrop : public CGameObject
{
public:
	CCrop();
	~CCrop();

	virtual void Update() override;

	virtual void CropStart(int8 CropStep);
	
	virtual bool OnDamaged(CGameObject* Attacker, int32 Damage) override;

	virtual void UpdateIdle() override;
	virtual void UpdateReadyDead() override;
	virtual void UpdateDead() override;
private:
	int8 _CropStep;
};

