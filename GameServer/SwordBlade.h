#pragma once
#include "GameObject.h"

class CSwordBlade : public CGameObject
{
public:
	CSwordBlade();
	~CSwordBlade();

	virtual void Update() override;

	void Move();	
private:	
};

