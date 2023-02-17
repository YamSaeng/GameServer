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
			_LeftTopX = 0.4f;
			_LeftTopY = 0.5f;

			_RightDownX = 0.4f;
			_RightDownY = 0.5f;
			break;
		case en_GameObjectType::OBJECT_GOBLIN:
			_LeftTopX = 0.4f;
			_LeftTopY = 0.5f;

			_RightDownX = 0.4f;
			_RightDownY = 0.5f;
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

void CRectCollision::CollisionUpdate()
{
	if (_Owner != nullptr)
	{
		_LeftTop._X = _Owner->_GameObjectInfo.ObjectPositionInfo.Position._X - _LeftTopX;
		_LeftTop._Y = _Owner->_GameObjectInfo.ObjectPositionInfo.Position._Y - _LeftTopY;

		_RightDown._X = _Owner->_GameObjectInfo.ObjectPositionInfo.Position._X + _RightDownX;
		_RightDown._Y = _Owner->_GameObjectInfo.ObjectPositionInfo.Position._Y + _RightDownY;							
	}
}

bool CRectCollision::IsCollision(CRectCollision* ARectCollision, CRectCollision* BRectCollision)
{
	if (ARectCollision->_RightDown._X < BRectCollision->_LeftTop._X
		|| ARectCollision->_LeftTop._X > BRectCollision->_RightDown._X)
	{
		return false;
	}

	if (ARectCollision->_RightDown._Y < BRectCollision->_LeftTop._Y
		|| ARectCollision->_LeftTop._Y > BRectCollision->_RightDown._Y)
	{
		return false;
	}

	return true;
}

void CRectCollision::SetActive(bool _Active)
{
	_RectCollisionActive = _Active;
}

bool CRectCollision::GetActive()
{
	return _RectCollisionActive;
}
