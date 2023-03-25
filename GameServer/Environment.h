#pragma once
#include"GameObject.h"

class CEnvironment : public CGameObject
{
public:
	CEnvironment();

	virtual void Start() override;

	virtual void Update() override;
	virtual bool OnDamaged(CGameObject* Attacker, int32 Damage) override;
protected:
	//------------------------
	// Idle 상태 Update
	//------------------------
	virtual void UpdateIdle() override;	
	//------------------------
	// Dead 상태 Update
	//------------------------
	virtual void UpdateDead() override;
};

class CStone : public CEnvironment
{
public:
	CStone();
	
	virtual void Start() override;
	virtual bool OnDamaged(CGameObject* Attacker, int32 Damage) override;
private:
	virtual void UpdateIdle() override;
};

class CTree : public CEnvironment
{
public:
	CTree();
	
	virtual void Start() override;
	virtual bool OnDamaged(CGameObject* Attacker, int32 Damage) override;
private:
	virtual void UpdateIdle() override;
};