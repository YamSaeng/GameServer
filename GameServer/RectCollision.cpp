#include "pch.h"
#include "RectCollision.h"
#include "GameObject.h"

CRectCollision::CRectCollision()
{

}

CRectCollision::~CRectCollision()
{

}

void CRectCollision::Init(CGameObject* OwnerObject)
{
	_RectCollisionActive = true;

	_OwnerObject = OwnerObject;

	if (_OwnerObject != nullptr)
	{
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

	Update();
}

void CRectCollision::Update()
{
	if (_OwnerObject != nullptr)
	{
		_Position = _OwnerObject->_GameObjectInfo.ObjectPositionInfo.Position;
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
