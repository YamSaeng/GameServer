#pragma once
#include "GameObject.h"

class CCrop : public CGameObject
{
public:
	CCrop();
	~CCrop();

	virtual void Update() override;

private:
	int8 _CropStep;
};

