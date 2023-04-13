#include "pch.h"
#include "RectCollision.h"
#include "GameObject.h"
#include "Matrix2x2.h"

CRectCollision::CRectCollision()
{

}

CRectCollision::~CRectCollision()
{

}

void CRectCollision::Init(CGameObject* OwnerObject, Vector2 Direction)
{
	_RectCollisionActive = true;

	_OwnerObject = OwnerObject;

	if (_OwnerObject != nullptr)
	{
		_Direction = Direction;

		switch (_OwnerObject->_GameObjectInfo.ObjectType)
		{
		case en_GameObjectType::OBJECT_PLAYER:
		case en_GameObjectType::OBJECT_PLAYER_DUMMY:
		case en_GameObjectType::OBJECT_NON_PLAYER_GENERAL_MERCHANT:
			_Size.X = 1.0f;
			_Size.Y = 1.0f;
			break;
		case en_GameObjectType::OBJECT_WALL:
			_Size.X = 1.0f;
			_Size.Y = 1.0f;
			break;
		case en_GameObjectType::OBJECT_GOBLIN:
			_Size.X = 1.0f;
			_Size.Y = 1.0f;
			break;
		case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
			_Size.X = 2.0f;
			_Size.Y = 2.0f;
			break;
		case en_GameObjectType::OBJECT_SKILL_SWORD_BLADE:
			_Size.X = 1.0f;
			_Size.Y = 0.5f;
			break;
		}

		PositionUpdate();
		RotateUpdate();		
	}
	else
	{
		CRASH("OwnerObject null");
	}
}

bool CRectCollision::IsCollision(CRectCollision* ARectCollision, CRectCollision* BRectCollision)
{
	return (ARectCollision->_Position.X < BRectCollision->_Position.X + BRectCollision->_Size.X
		&& ARectCollision->_Position.X + ARectCollision->_Size.X > BRectCollision->_Position.X
		&& ARectCollision->_Position.Y > BRectCollision->_Position.Y - BRectCollision->_Size.Y
		&& ARectCollision->_Position.Y - ARectCollision->_Size.Y < BRectCollision->_Position.Y);
}

void CRectCollision::SetActive(bool _Active)
{
	_RectCollisionActive = _Active;
}

void CRectCollision::PositionUpdate()
{
	_Position = _OwnerObject->_GameObjectInfo.ObjectPositionInfo.Position;

	_MiddlePosition.X = _Position.X + _Size.X / 2.0f;
	_MiddlePosition.Y = _Position.Y - _Size.Y / 2.0f;

	_LeftTop = _Position;

	_LeftDown.X = _Position.X;
	_LeftDown.Y = _Position.Y - _Size.Y;

	_RightTop.X = _Position.X + _Size.X;
	_RightTop.Y = _Position.Y;

	_RightDown.X = _Position.X + _Size.X;
	_RightDown.Y = _Position.Y - _Size.Y;
}

void CRectCollision::RotateUpdate()
{
	float Degree = _Direction.AngleToDegree();
	float Sin = 0.0f; float Cos = 0.0f;
	Math::GetSinCos(Sin, Cos, Degree);

	// 회전 행렬 생성
	Vector2 Basis1(Cos, Sin);
	Vector2 Basis2(-Sin, Cos);
	Matrix2x2 RotationMatrix(Basis1, Basis2);

	// 중심 좌표 기준으로 회전
	Vector2 LeftTopRot = RotationMatrix * (_LeftTop - _MiddlePosition);
	Vector2 LeftDown = RotationMatrix * (_LeftDown - _MiddlePosition);
	Vector2 RightTop = RotationMatrix * (_RightTop - _MiddlePosition);
	Vector2 RightDown = RotationMatrix * (_RightDown - _MiddlePosition);

	// 중심 좌표로 이동
	_LeftTop = LeftTopRot + _MiddlePosition;
	_LeftDown = LeftDown + _MiddlePosition;
	_RightTop = RightTop + _MiddlePosition;
	_RightDown = RightDown + _MiddlePosition;
}

void CRectCollision::Update()
{
	if (_OwnerObject != nullptr)
	{
		PositionUpdate();
		RotateUpdate();		
	}
	else
	{
		CRASH("OwnerObject null");
	}
}

bool CRectCollision::GetActive()
{
	return _RectCollisionActive;
}
