#include "pch.h"
#include "RectCollision.h"
#include "GameObject.h"

CRectCollision::CRectCollision()
{

}

CRectCollision::CRectCollision(CGameObject* Owner)
{
	_RectCollisionActive = true;

	_Owner = Owner;	

	if (_Owner != nullptr)
	{
		switch (_Owner->_GameObjectInfo.ObjectType)
		{
		case en_GameObjectType::OBJECT_PLAYER:
		case en_GameObjectType::OBJECT_PLAYER_DUMMY:
			_Size._X = 1.0f;
			_Size._Y = 1.0f;
			break;
		case en_GameObjectType::OBJECT_LEFT_RIGHT_WALL:
		case en_GameObjectType::OBJECT_UP_DOWN_WALL:
		case en_GameObjectType::OBJECT_UP_TO_LEFT_WALL:
		case en_GameObjectType::OBJECT_UP_TO_RIGHT_WALL:
		case en_GameObjectType::OBJECT_DOWN_TO_LEFT_WALL:
		case en_GameObjectType::OBJECT_DOWN_TO_RIGHT_WALL:
			_Size._X = 1.0f;
			_Size._Y = 1.0f;
			break;
		case en_GameObjectType::OBJECT_GOBLIN:
			_Size._X = 1.0f;
			_Size._Y = 1.0f;
			break;
		case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
			_Size._X = 2.0f;
			_Size._Y = 2.0f;
			break;
		}
	}
}

CRectCollision::~CRectCollision()
{

}

void CRectCollision::Init()
{

}

bool CRectCollision::IsCollision(CRectCollision* ARectCollision, CRectCollision* BRectCollision)
{
	return (ARectCollision->_Position._X < BRectCollision->_Position._X + BRectCollision->_Size._X
		&& ARectCollision->_Position._X + ARectCollision->_Size._X > BRectCollision->_Position._X
		&& ARectCollision->_Position._Y > BRectCollision->_Position._Y - BRectCollision->_Size._Y
		&& ARectCollision->_Position._Y - ARectCollision->_Size._Y < BRectCollision->_Position._Y);
}

void CRectCollision::SetActive(bool _Active)
{
	_RectCollisionActive = _Active;
}

void CRectCollision::Update()
{
	_Position = _Owner->_GameObjectInfo.ObjectPositionInfo.Position;
}

bool CRectCollision::GetActive()
{
	return _RectCollisionActive;
}
