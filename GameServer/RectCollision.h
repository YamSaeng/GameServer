#pragma once
#include "GameObjectInfo.h"

class CGameObject;

class CRectCollision
{
public:
	en_CollisionPosition _CollisionPosition;		
	
	Vector2 _Size;

	Vector2 _Direction;	

	Vector2 _Position;

	Vector2 _LeftTop;
	Vector2 _RightTop;
	Vector2 _LeftDown;
	Vector2 _RightDown;

	CRectCollision();
	~CRectCollision();

	void ObjectRectInit(en_CollisionPosition CollisionPosition, en_GameObjectType ObjectType, Vector2 InitPosition, Vector2 Direction, CGameObject* OwnerObject = nullptr);
	void SkillRectInit(en_CollisionPosition CollisionPosition, CSkill* Skill, Vector2 InitPosition, Vector2 Direction,  CGameObject* OwnerObject = nullptr);
	void LeftTopRightDownRectInit(Vector2 LeftTopPosition, Vector2 RightDownPosition);	

	static bool IsCollision(CRectCollision* ARectCollision, CRectCollision* BRectCollision);
	static bool IsOBBCollision(CRectCollision* ARectCollision, CRectCollision* BRectCollision);

	static Vector2 GetHeightVector(CRectCollision* RectCollision);
	static Vector2 GetWidthVector(CRectCollision* RectCollision);

	bool GetActive();
	void SetActive(bool _Active);

	void PositionUpdate();
	void NotSetPositionUpdate();
	void RotateUpdate();

	void Update();

	// 소유 대상이 죽을 경우 크기 등 변경
	void DeadPositionUpdate();
private:
	CGameObject* _OwnerObject;

	bool _RectCollisionActive;	
};

