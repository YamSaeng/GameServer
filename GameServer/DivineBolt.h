#pragma once
#include "GameObject.h"

class CDivineBolt : public CGameObject
{
public:
	CDivineBolt();
	~CDivineBolt();

	virtual void Update() override;

	void Move();
private:
};

