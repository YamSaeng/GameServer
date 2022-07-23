#pragma once
#include "GameObjectInfo.h"

class CGameObject;

class CRectCollision
{
public:	
	st_Vector2 _LeftTop;
	st_Vector2 _RightDown;

	CRectCollision();
	CRectCollision(CGameObject* Owner);
	~CRectCollision();

	void Init();

	void CollisionUpdate();
	static bool IsCollision(CRectCollision* ARectCollision, CRectCollision* BRectCollision);
	bool GetActive();
	void SetActive(bool _Active);	
private:
	CGameObject* _Owner;

	bool _RectCollisionActive;
};