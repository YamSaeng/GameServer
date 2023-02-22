#pragma once
#include "GameObjectInfo.h"

class CGameObject;

class CRectCollision
{
public:	
	st_Vector2 _Position;
	st_Vector2 _Size;	

	CRectCollision();
	CRectCollision(CGameObject* Owner);
	~CRectCollision();

	void Init();

	static bool IsCollision(CRectCollision* ARectCollision, CRectCollision* BRectCollision);

	bool GetActive();
	void SetActive(bool _Active);	

	void Update();
private:
	CGameObject* _Owner;

	bool _RectCollisionActive;
};