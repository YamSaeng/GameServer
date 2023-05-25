#pragma once
#include "GameObject.h"

class CFlameBolt : public CGameObject
{
public:
	CFlameBolt();
	~CFlameBolt();
	
	virtual void Update() override;

	void Move();
private:
};

