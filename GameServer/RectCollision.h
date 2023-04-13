#pragma once
#include "GameObjectInfo.h"

class CGameObject;

class CRectCollision
{
public:
	Vector2 _Position;
	Vector2 _Size;

	Vector2 _Direction;

	Vector2 _MiddlePosition;

	Vector2 _LeftTop;
	Vector2 _RightTop;
	Vector2 _LeftDown;
	Vector2 _RightDown;

	CRectCollision();
	~CRectCollision();

	void Init(CGameObject* OwnerObject, Vector2 Direction);

	static bool IsCollision(CRectCollision* ARectCollision, CRectCollision* BRectCollision);

	bool GetActive();
	void SetActive(bool _Active);

	void PositionUpdate();
	void RotateUpdate();
	void Update();
private:
	CGameObject* _OwnerObject;

	bool _RectCollisionActive;
};