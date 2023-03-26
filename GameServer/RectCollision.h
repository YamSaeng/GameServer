#pragma once
#include "GameObjectInfo.h"

class CGameObject;

class CRectCollision
{
public:	
	st_Vector2 _Position;
	st_Vector2 _Size;	

	CRectCollision();	
	~CRectCollision();

	void Init(CGameObject* OwnerObject);

	static bool IsCollision(CRectCollision* ARectCollision, CRectCollision* BRectCollision);

	bool GetActive();
	void SetActive(bool _Active);	

	void Update();
private:
	CGameObject* _OwnerObject;

	bool _RectCollisionActive;
};