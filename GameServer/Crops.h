#pragma once
#include "GameObject.h"

class Crops : public CGameObject
{
public:
	Crops();
	~Crops();

	virtual void Update() override;

private:
	int8 _CropStep;
};

