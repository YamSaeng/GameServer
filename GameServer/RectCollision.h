#pragma once
#include "GameObjectInfo.h"

class CGameObject;

class CRectCollision
{
public:
	en_CollisionPosition _CollisionPosition;
		
	Vector2 _Size;

	Vector2 _Direction;

	Vector2 _MiddlePosition;

	Vector2 _Position;

	Vector2 _LeftTop;
	Vector2 _RightTop;
	Vector2 _LeftDown;
	Vector2 _RightDown;

	CRectCollision();
	~CRectCollision();

	void Init(en_CollisionPosition CollisionPosition, en_GameObjectType ObjectType, Vector2 InitPosition, Vector2 Direction, CGameObject* OwnerObject = nullptr);
	void Init(en_CollisionPosition CollisionPosition, en_SkillType SkillType, Vector2 InitPosition, Vector2 Direction, CGameObject* OwnerObject = nullptr);

	static bool IsCollision(CRectCollision* ARectCollision, CRectCollision* BRectCollision);
	static bool IsOBBCollision(CRectCollision* ARectCollision, CRectCollision* BRectCollision);

	static Vector2 GetHeightVector(CRectCollision* RectCollision);
	static Vector2 GetWidthVector(CRectCollision* RectCollision);

	bool GetActive();
	void SetActive(bool _Active);

	void PositionUpdate();
	void RotateUpdate();

	void Update();
private:
	CGameObject* _OwnerObject;

	bool _RectCollisionActive;
};

