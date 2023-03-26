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
			_Size._X = 1.0f;
			_Size._Y = 1.0f;
			break;
		case en_GameObjectType::OBJECT_WALL:
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
	else
	{
		CRASH("OwnerObject null");
	}
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
