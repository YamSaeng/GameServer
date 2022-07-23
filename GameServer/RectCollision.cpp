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
		switch (_Owner->_GameObjectInfo.ObjectType)
		{
		case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
		case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
		case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
		case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
		case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL:
			_LeftTop._X = _Owner->_GameObjectInfo.ObjectPositionInfo.Position._X - 0.4f;
			_LeftTop._Y = _Owner->_GameObjectInfo.ObjectPositionInfo.Position._Y - 0.4f;

			_RightDown._X = _Owner->_GameObjectInfo.ObjectPositionInfo.Position._X + 0.4f;
			_RightDown._Y = _Owner->_GameObjectInfo.ObjectPositionInfo.Position._Y + 0.4f;
			break;
		case en_GameObjectType::OBJECT_SLIME:
			_LeftTop._X = _Owner->_GameObjectInfo.ObjectPositionInfo.Position._X - 0.3f;
			_LeftTop._Y = _Owner->_GameObjectInfo.ObjectPositionInfo.Position._Y - 0.3f;

			_RightDown._X = _Owner->_GameObjectInfo.ObjectPositionInfo.Position._X + 0.3f;
			_RightDown._Y = _Owner->_GameObjectInfo.ObjectPositionInfo.Position._Y + 0.3f;
			break;
		}		

		/*G_Logger->WriteStdOut(en_Color::RED, L"X : %0.2f Y : %0.2f Collision LeftTopX : %0.2f LeftTopY : %0.2f RightDownX : %0.2f RightDownY : %0.2f\n"
			, _Owner->_GameObjectInfo.ObjectPositionInfo.Position._X, _Owner->_GameObjectInfo.ObjectPositionInfo.Position._Y,
			_LeftTop._X, _LeftTop._Y, _RightDown._X, _RightDown._Y);*/
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
