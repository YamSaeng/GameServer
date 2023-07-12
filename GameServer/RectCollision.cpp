
#include "pch.h"
#include "RectCollision.h"
#include "Creature.h"
#include "EquipmentBox.h"
#include "Matrix2x2.h"
#include "Skill.h"

CRectCollision::CRectCollision()
{

}

CRectCollision::~CRectCollision()
{

}

void CRectCollision::Init(en_CollisionPosition CollisionPosition, en_GameObjectType ObjectType, Vector2 InitPosition, Vector2 Direction, CGameObject* OwnerObject)
{
	_RectCollisionActive = true;	

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
	case en_GameObjectType::OBJECT_SKILL_FLAME_BOLT:
		_Size.X = 1.0f;
		_Size.Y = 0.7f;
		break;
	case en_GameObjectType::OBJECT_SKILL_DIVINE_BOLT:
		_Size.X = 1.0f;
		_Size.Y = 0.7f;
		break;
	}	

	_OwnerObject = OwnerObject;	

	_Direction = Direction;	

	_CollisionPosition = CollisionPosition;

	PositionUpdate();
	RotateUpdate();
}

void CRectCollision::SkillRectInit(en_CollisionPosition CollisionPosition, CSkill* Skill, Vector2 InitPosition, Vector2 Direction, CGameObject* OwnerObject)
{
	_RectCollisionActive = true;

	_CollisionPosition = CollisionPosition;

	_OwnerObject = OwnerObject;

	_Direction = Direction;			

	CCreature* Creature = dynamic_cast<CCreature*>(_OwnerObject);	

	switch (_CollisionPosition)
	{		
	case en_CollisionPosition::COLLISION_POSITION_SKILL_MIDDLE:
		_Position = InitPosition;
		
		break;
	case en_CollisionPosition::COLLISION_POSITION_SKILL_FRONT:
		_Position = InitPosition + _Direction;

		break;	
	}

	if (Creature != nullptr)
	{
		CEquipmentBox* Equipment = Creature->GetEquipment();
		if (Equipment != nullptr)
		{
			CItem* RightHandItem = Equipment->GetEquipmentParts(en_EquipmentParts::EQUIPMENT_PARTS_RIGHT_HAND);
			if (RightHandItem != nullptr)
			{
				if (RightHandItem->_ItemInfo.ItemSmallCategory != en_SmallItemCategory::ITEM_SMALL_CATEGORY_NONE)
				{
					_Size.X = RightHandItem->_ItemInfo.ItemCollisionX + Skill->GetSkillInfo()->SkillRangeX;
					_Size.Y = RightHandItem->_ItemInfo.ItemCollisionY + Skill->GetSkillInfo()->SkillRangeY;
				}
			}
		}
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
	Vector2 AMiddlePosition;
	AMiddlePosition.X = ((ARectCollision->_LeftTop.X * ARectCollision->_RightDown.Y - ARectCollision->_LeftTop.Y * ARectCollision->_RightDown.X) * (ARectCollision->_LeftDown.X - ARectCollision->_RightTop.X) - (ARectCollision->_LeftTop.X - ARectCollision->_RightDown.X) * (ARectCollision->_LeftDown.X * ARectCollision->_RightTop.Y - ARectCollision->_LeftDown.Y * ARectCollision->_RightTop.X)) / ((ARectCollision->_LeftTop.X - ARectCollision->_RightDown.X) * (ARectCollision->_LeftDown.Y - ARectCollision->_RightTop.Y) - (ARectCollision->_LeftTop.Y - ARectCollision->_RightDown.Y) * (ARectCollision->_LeftDown.X - ARectCollision->_RightTop.X));
	AMiddlePosition.Y = ((ARectCollision->_LeftTop.X * ARectCollision->_RightDown.Y - ARectCollision->_LeftTop.Y * ARectCollision->_RightDown.X) * (ARectCollision->_LeftDown.Y - ARectCollision->_RightTop.Y) - (ARectCollision->_LeftTop.Y - ARectCollision->_RightDown.Y) * (ARectCollision->_LeftDown.X * ARectCollision->_RightTop.Y - ARectCollision->_LeftDown.Y * ARectCollision->_RightTop.X)) / ((ARectCollision->_LeftTop.X - ARectCollision->_RightDown.X) * (ARectCollision->_LeftDown.Y - ARectCollision->_RightTop.Y) - (ARectCollision->_LeftTop.Y - ARectCollision->_RightDown.Y) * (ARectCollision->_LeftDown.X - ARectCollision->_RightTop.X));

	Vector2 BMiddlePosition;
	BMiddlePosition.X = ((BRectCollision->_LeftTop.X * BRectCollision->_RightDown.Y - BRectCollision->_LeftTop.Y * BRectCollision->_RightDown.X) * (BRectCollision->_LeftDown.X - BRectCollision->_RightTop.X) - (BRectCollision->_LeftTop.X - BRectCollision->_RightDown.X) * (BRectCollision->_LeftDown.X * BRectCollision->_RightTop.Y - BRectCollision->_LeftDown.Y * BRectCollision->_RightTop.X)) / ((BRectCollision->_LeftTop.X - BRectCollision->_RightDown.X) * (BRectCollision->_LeftDown.Y - BRectCollision->_RightTop.Y) - (BRectCollision->_LeftTop.Y - BRectCollision->_RightDown.Y) * (BRectCollision->_LeftDown.X - BRectCollision->_RightTop.X));
	BMiddlePosition.Y = ((BRectCollision->_LeftTop.X * BRectCollision->_RightDown.Y - BRectCollision->_LeftTop.Y * BRectCollision->_RightDown.X) * (BRectCollision->_LeftDown.Y - BRectCollision->_RightTop.Y) - (BRectCollision->_LeftTop.Y - BRectCollision->_RightDown.Y) * (BRectCollision->_LeftDown.X * BRectCollision->_RightTop.Y - BRectCollision->_LeftDown.Y * BRectCollision->_RightTop.X)) / ((BRectCollision->_LeftTop.X - BRectCollision->_RightDown.X) * (BRectCollision->_LeftDown.Y - BRectCollision->_RightTop.Y) - (BRectCollision->_LeftTop.Y - BRectCollision->_RightDown.Y) * (BRectCollision->_LeftDown.X - BRectCollision->_RightTop.X));

	Vector2 ABCenterVector;
	ABCenterVector = AMiddlePosition - BMiddlePosition;	

	Vector2 StandardVector[4] = { GetWidthVector(ARectCollision), GetHeightVector(ARectCollision), GetWidthVector(BRectCollision) , GetHeightVector(BRectCollision) };

	Vector2 StandardVectorNormal;

	float ARectAngle = ARectCollision->_Direction.AngleToDegree();
	float BRectAngle = BRectCollision->_Direction.AngleToDegree();

	for (int i = 0; i < 4; i++)
	{
		float Sum = 0;
		StandardVectorNormal = StandardVector[i].Normalize();

		float ARectWidthDot = StandardVector[0].ABSDot(StandardVectorNormal);
		float ARectHeightDot = StandardVector[1].ABSDot(StandardVectorNormal);
		float BRectWidthDot = StandardVector[2].ABSDot(StandardVectorNormal);
		float BRectHeightDot = StandardVector[3].ABSDot(StandardVectorNormal);						

		Sum = ARectWidthDot + ARectHeightDot + BRectWidthDot + BRectHeightDot;		

		float Distance = ABCenterVector.ABSDot(StandardVectorNormal);			
		if (Distance > Sum)
		{
			return false;
		}
	}

	return true;
}

Vector2 CRectCollision::GetHeightVector(CRectCollision* RectCollision)
{
	Vector2 HeightVector;	

	HeightVector.X = RectCollision->_Size.Y * cos(Math::DegreeToRadian(RectCollision->_Direction.AngleToDegree() - 90)) / 2.0f;
	HeightVector.Y = RectCollision->_Size.Y * sin(Math::DegreeToRadian(RectCollision->_Direction.AngleToDegree() - 90)) / 2.0f;

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
	switch (_CollisionPosition)
	{	
	case en_CollisionPosition::COLLISION_POSITION_OBJECT:
		if (_OwnerObject != nullptr)
		{
			_Position = _OwnerObject->_GameObjectInfo.ObjectPositionInfo.Position;
		}
		break;
	case en_CollisionPosition::COLLISION_POSITION_SKILL_MIDDLE:
		break;
	case en_CollisionPosition::COLLISION_POSITION_SKILL_FRONT:
		break;	
	}	
	
	_LeftTop.X = _Position.X - _Size.X / 2.0f;
	_LeftTop.Y = _Position.Y + _Size.Y / 2.0f;

	_RightTop.X = _Position.X + _Size.X / 2.0f;
	_RightTop.Y = _Position.Y + _Size.Y / 2.0f;

	_LeftDown.X = _Position.X - _Size.X / 2.0f;
	_LeftDown.Y = _Position.Y - _Size.Y / 2.0f;

	_RightDown.X = _Position.X + _Size.X / 2.0f;
	_RightDown.Y = _Position.Y - _Size.Y / 2.0f;	
}

void CRectCollision::NotSetPositionUpdate()
{
	_LeftTop.X = _Position.X - _Size.X / 2.0f;
	_LeftTop.Y = _Position.Y + _Size.Y / 2.0f;

	_RightTop.X = _Position.X + _Size.X / 2.0f;
	_RightTop.Y = _Position.Y + _Size.Y / 2.0f;

	_LeftDown.X = _Position.X - _Size.X / 2.0f;
	_LeftDown.Y = _Position.Y - _Size.Y / 2.0f;

	_RightDown.X = _Position.X + _Size.X / 2.0f;
	_RightDown.Y = _Position.Y - _Size.Y / 2.0f;
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
	Vector2 LeftTopRot = RotationMatrix * (_LeftTop - _Position);
	Vector2 LeftDownRot = RotationMatrix * (_LeftDown - _Position);
	Vector2 RightTopRot = RotationMatrix * (_RightTop - _Position);
	Vector2 RightDownRot = RotationMatrix * (_RightDown - _Position);

	// 중심 좌표로 이동
	_LeftTop   = LeftTopRot + _Position;
	_LeftDown  = LeftDownRot + _Position;
	_RightTop  = RightTopRot + _Position;
	_RightDown = RightDownRot + _Position;
}

void CRectCollision::Update()
{
	PositionUpdate();
	RotateUpdate();
}

void CRectCollision::DeadPositionUpdate()
{	
	if (_OwnerObject != nullptr)
	{
		switch (_OwnerObject->_GameObjectInfo.ObjectType)
		{
		case en_GameObjectType::OBJECT_GOBLIN:
			_Size.X = 1.0f;
			_Size.Y = 0.7f;			
			break;		
		}
	}	
}

bool CRectCollision::GetActive()
{
	return _RectCollisionActive;
}