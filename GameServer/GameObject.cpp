#include "pch.h"
#include "GameObject.h"

CGameObject::CGameObject()
{
	_Channel = nullptr;
}

CGameObject::CGameObject(st_GameObjectInfo GameObjectInfo)
{
	_GameObjectInfo = GameObjectInfo;	
}

CGameObject::~CGameObject()
{

}

void CGameObject::Update()
{

}

void CGameObject::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	_GameObjectInfo.ObjectStatInfo.HP -= Damage;

	if (_GameObjectInfo.ObjectStatInfo.HP <= 0)
	{
		_GameObjectInfo.ObjectStatInfo.HP = 0;
	}
}

void CGameObject::OnDead(CGameObject* Killer)
{

}

st_PositionInfo CGameObject::GetPositionInfo()
{
	return _GameObjectInfo.ObjectPositionInfo;
}

st_Vector2Int CGameObject::GetCellPosition()
{
	return st_Vector2Int(_GameObjectInfo.ObjectPositionInfo.PositionX,_GameObjectInfo.ObjectPositionInfo.PositionY);
}

st_Vector2Int CGameObject::GetFrontCellPosition(en_MoveDir Dir)
{
	st_Vector2Int FrontPosition = GetCellPosition();

	st_Vector2Int DirVector;
	switch (Dir)
	{
	case en_MoveDir::UP:		
		DirVector = st_Vector2Int::Up();
		break;
	case en_MoveDir::DOWN:
		DirVector = st_Vector2Int::Down();
		break;
	case en_MoveDir::LEFT:
		DirVector = st_Vector2Int::Left();
		break;
	case en_MoveDir::RIGHT:
		DirVector = st_Vector2Int::Right();
		break;	
	}

	FrontPosition = FrontPosition + DirVector;

	return FrontPosition;
}

en_MoveDir CGameObject::GetDirectionFromVector(st_Vector2Int DirectionVector)
{
	if (DirectionVector._X > 0)
	{
		return en_MoveDir::RIGHT;
	}
	else if (DirectionVector._X < 0)
	{
		return en_MoveDir::LEFT;
	}
	else if (DirectionVector._Y > 0)
	{
		return en_MoveDir::UP;
	}
	else
	{
		return en_MoveDir::DOWN;
	}
}
