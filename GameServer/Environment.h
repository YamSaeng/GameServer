#pragma once
#include"GameObject.h"

class CEnvironment : public CGameObject
{
public:
	CEnvironment();

	virtual void Init(st_Vector2Int SpawnPosition);

	virtual void Update() override;
	virtual void OnDamaged(CGameObject* Attacker, int32 Damage) override;
protected:
	//------------------------
	// Idle ป๓ลย Update
	//------------------------
	virtual void UpdateIdle();
};

class CStone : public CEnvironment
{
public:
	CStone();
	
	virtual void Init(st_Vector2Int SpawnPosition) override;
	virtual void OnDead(CGameObject* Killer) override;
private:
	virtual void UpdateIdle() override;
};

class CTree : public CEnvironment
{
public:
	CTree();
	
	virtual void Init(st_Vector2Int SpawnPosition) override;
	virtual void OnDead(CGameObject* Killer) override;
private:
	virtual void UpdateIdle() override;
};
