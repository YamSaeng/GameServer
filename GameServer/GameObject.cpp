#include "pch.h"
#include "GameObject.h"

CGameObject::CGameObject()
{
	_NetworkState = en_ObjectNetworkState::READY;
	_GameObjectInfo.OwnerObjectId = 0;
	_Channel = nullptr;
}

CGameObject::CGameObject(st_GameObjectInfo GameObjectInfo)
{
	_GameObjectInfo = GameObjectInfo;
	_NetworkState = en_ObjectNetworkState::READY;
	_GameObjectInfo.OwnerObjectId = 0;
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
	return st_Vector2Int(_GameObjectInfo.ObjectPositionInfo.PositionX, _GameObjectInfo.ObjectPositionInfo.PositionY);
}

st_Vector2Int CGameObject::GetFrontCellPosition(en_MoveDir Dir, int8 Distance)
{
	st_Vector2Int FrontPosition = GetCellPosition();

	st_Vector2Int DirVector;
	switch (Dir)
	{
	case en_MoveDir::UP:
		DirVector = st_Vector2Int::Up() * Distance;
		break;
	case en_MoveDir::DOWN:
		DirVector = st_Vector2Int::Down() * Distance;
		break;
	case en_MoveDir::LEFT:
		DirVector = st_Vector2Int::Left() * Distance;
		break;
	case en_MoveDir::RIGHT:
		DirVector = st_Vector2Int::Right() * Distance;
		break;
	}

	FrontPosition = FrontPosition + DirVector;

	return FrontPosition;
}

vector<st_Vector2Int> CGameObject::GetAroundCellPosition(st_Vector2Int CellPosition, int8 Distance)
{
	vector<st_Vector2Int> AroundPosition;

	st_Vector2Int LeftTop(Distance * -1, Distance);
	st_Vector2Int RightDown(Distance, Distance * -1);

	st_Vector2Int LeftTopPosition = CellPosition + LeftTop;
	st_Vector2Int RightDownPosition = CellPosition + RightDown;

	for (int32 Y = LeftTopPosition._Y; Y >= RightDownPosition._Y; Y--)
	{
		for (int32 X = LeftTopPosition._X; X <= RightDownPosition._X; X++)
		{
			if (X == CellPosition._X && Y == CellPosition._Y)
			{
				continue;
			}

			AroundPosition.push_back(st_Vector2Int(X, Y));
		}
	}

	return AroundPosition;
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
