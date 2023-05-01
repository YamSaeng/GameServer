
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

void CRectCollision::Init(en_CollisionPosition CollisionPosition, en_GameObjectType ObjectType, Vector2 InitPosition, Vector2 Direction, CGameObject* OwnerObject)
{
	_RectCollisionActive = true;

	_OwnerObject = OwnerObject;

	_LeftTop = InitPosition;

	_Direction = Direction;

	switch (ObjectType)
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

void CRectCollision::Init(en_CollisionPosition CollisionPosition, en_SkillType SkillType, Vector2 InitPosition, Vector2 Direction, CGameObject* OwnerObject)
{
	_RectCollisionActive = true;

	_CollisionPosition = CollisionPosition;

	_OwnerObject = OwnerObject;

	_Position = InitPosition;

	_LeftTop = InitPosition;

	_Direction = Direction;

	switch (SkillType)
	{	
	case en_SkillType::SKILL_DEFAULT_ATTACK:
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FIERCE_ATTACK:
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK:
		_Size.X = 1.2f;
		_Size.Y = 1.4f;
		break;		
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_SMASH_WAVE:
		_Size.X = 3.0f;
		_Size.Y = 3.0f;
		break;
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_PIERCING_WAVE:
		_Size.X = 5.0f;
		_Size.Y = 5.0f;
		break;
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_JUMPING_ATTACK:
		_Size.X = 3.0f;
		_Size.Y = 3.0f;
		break;
	case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SWORD_STORM:
		_Size.X = 3.0f;
		_Size.Y = 1.5f;
		break;
	}

	PositionUpdate();
	RotateUpdate();
}

bool CRectCollision::IsCollision(CRectCollision* ARectCollision, CRectCollision* BRectCollision)
{
	return (ARectCollision->_LeftTop.X < BRectCollision->_LeftTop.X + BRectCollision->_Size.X
		&& ARectCollision->_LeftTop.X + ARectCollision->_Size.X > BRectCollision->_LeftTop.X
		&& ARectCollision->_LeftTop.Y > BRectCollision->_LeftTop.Y - BRectCollision->_Size.Y
		&& ARectCollision->_LeftTop.Y - ARectCollision->_Size.Y < BRectCollision->_LeftTop.Y);
}

bool CRectCollision::IsOBBCollision(CRectCollision* ARectCollision, CRectCollision* BRectCollision)
{	
	Vector2 OBBUnit;
	OBBUnit.X = (ARectCollision->_Position.X + ARectCollision->_Size.X / 2) - (BRectCollision->_LeftTop.X + BRectCollision->_Size.X / 2);
	OBBUnit.Y = (ARectCollision->_Position.Y - ARectCollision->_Size.Y / 2) - (BRectCollision->_LeftTop.Y - BRectCollision->_Size.Y / 2);

	float Angle = ARectCollision->_Direction.AngleToDegree();

	Vector2 StandardVector[4] = { GetHeightVector(ARectCollision), GetHeightVector(BRectCollision),
	GetWidthVector(ARectCollision), GetWidthVector(BRectCollision) };	

	Vector2 StandardVectorNormal;

	for (int i = 0; i < 4; i++)
	{
		float Sum = 0;
		StandardVectorNormal = StandardVector[i].Normalize();
		for (int j = 0; j < 4; j++)
		{
			Sum += StandardVector[j].Dot(StandardVectorNormal);
		}

		if (OBBUnit.Dot(StandardVectorNormal) > Sum)
		{			
			return false;
		}
	}
	
	return true;
}

Vector2 CRectCollision::GetHeightVector(CRectCollision* RectCollision)
{
	Vector2 HeightVector;
	HeightVector.X = RectCollision->_Size.Y	* cos(Math::DegreeToRadian(RectCollision->_Direction.AngleToDegree() - 90)) / 2.0f;
	HeightVector.Y = RectCollision->_Size.Y	* sin(Math::DegreeToRadian(RectCollision->_Direction.AngleToDegree() - 90)) / 2.0f;

	return HeightVector;
}

Vector2 CRectCollision::GetWidthVector(CRectCollision* RectCollision)
{
	Vector2 WidthVector;
	WidthVector.X = RectCollision->_Size.X * cos(Math::DegreeToRadian(RectCollision->_Direction.AngleToDegree())) / 2.0f;
	WidthVector.Y = RectCollision->_Size.X * sin(Math::DegreeToRadian(RectCollision->_Direction.AngleToDegree())) / 2.0f;

	return WidthVector;
}

void CRectCollision::SetActive(bool _Active)
{
	_RectCollisionActive = _Active;
}

void CRectCollision::PositionUpdate()
{
	if (_OwnerObject != nullptr)
	{
		_Position = _OwnerObject->_GameObjectInfo.ObjectPositionInfo.Position;
		_LeftTop = _OwnerObject->_GameObjectInfo.ObjectPositionInfo.Position;
	}

	switch (_CollisionPosition)
	{
	case en_CollisionPosition::COLLISION_POSITION_MIDDLE:
		_LeftTop.X = _LeftTop.X - abs(0.5f * (_Size.X - 1));
		_LeftTop.Y = _LeftTop.Y + abs(0.5f * (_Size.Y - 1));		
		break;
	}

	_LeftDown.X = _LeftTop.X;
	_LeftDown.Y = _LeftTop.Y - _Size.Y;

	_RightTop.X = _LeftTop.X + _Size.X;
	_RightTop.Y = _LeftTop.Y;

	_RightDown.X = _LeftTop.X + _Size.X;
	_RightDown.Y = _LeftTop.Y - _Size.Y;	

	_MiddlePosition.X = _LeftTop.X + _Size.X / 2.0f;
	_MiddlePosition.Y = _LeftTop.Y - _Size.Y / 2.0f;
}

void CRectCollision::RotateUpdate()
{
	float Degree = _Direction.AngleToDegree();
	if (Degree == 0.0f)
	{
		return;
	}

	float Sin = 0.0f; float Cos = 0.0f;
	Math::GetSinCosDegree(Sin, Cos, Degree);

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
	PositionUpdate();
	RotateUpdate();
}

bool CRectCollision::GetActive()
{
	return _RectCollisionActive;
}