#pragma once
#include "FileUtils.h"
#include "GameObjectInfo.h"

class CItem;
class CGameObject;

struct st_Vector2
{
	float _X;
	float _Y;

	st_Vector2()
	{
		_X = 0;
		_Y = 0;
	}

	st_Vector2(float X, float Y)
	{
		_X = X;
		_Y = Y;
	}

	static st_Vector2 Up() { return st_Vector2(0, 1.0f); }
	static st_Vector2 Down() { return st_Vector2(0, -1.0f); }
	static st_Vector2 Left() { return st_Vector2(-1.0f, 0); }
	static st_Vector2 Right() { return st_Vector2(1.0f, 0); }
	static st_Vector2 Zero() { return st_Vector2(0, 0); }

	st_Vector2 operator + (st_Vector2& Vector)
	{
		return st_Vector2(_X + Vector._X, _Y + Vector._Y);
	}

	st_Vector2 operator - (st_Vector2& Vector)
	{
		return st_Vector2(_X - Vector._X, _Y - Vector._Y);
	}

	st_Vector2 operator * (int8& Sclar)
	{
		return st_Vector2(_X * Sclar, _Y * Sclar);
	}		

	// 거리 구하기
	static float Distance(st_Vector2 TargetCellPosition, st_Vector2 MyCellPosition)
	{
		float CalculateDistance = (float)sqrt(pow(TargetCellPosition._X - MyCellPosition._X, 2) + pow(TargetCellPosition._Y - MyCellPosition._Y, 2));
		return round((CalculateDistance * 100) / 100);
	}

	// 벡터 크기 구하기
	static float Size(st_Vector2 Vector)
	{
		return (float)sqrt(pow(Vector._X, 2) + pow(Vector._Y, 2));
	}

	// 벡터 정규화
	static st_Vector2 Normalize(st_Vector2 Vector)
	{
		float VectorSize = st_Vector2::Size(Vector);

		// 둘째 자리에서 반올림
		st_Vector2 NormalVector;
		NormalVector._X = round(((Vector._X / VectorSize) * 100) / 100);
		NormalVector._Y = round(((Vector._Y / VectorSize) * 100) / 100);

		return NormalVector;
	}

	static en_MoveDir GetMoveDir(st_Vector2 NormalVector)
	{
		if (NormalVector._X > 0)
		{
			return en_MoveDir::RIGHT;
		}

		if (NormalVector._X < 0)
		{
			return en_MoveDir::LEFT;
		}

		if (NormalVector._Y > 0)
		{
			return en_MoveDir::UP;
		}

		if (NormalVector._Y < 0)
		{
			return en_MoveDir::DOWN;
		}
	}
};

struct st_Vector2Int
{
	int32 _X;
	int32 _Y;

	st_Vector2Int()
	{
		_X = 0;
		_Y = 0;
	}

	st_Vector2Int(int32 X, int32 Y)
	{
		_X = X;
		_Y = Y;
	}

	// 방향 벡터
	static st_Vector2Int Up() { return st_Vector2Int(0, 1); }
	static st_Vector2Int Down() { return st_Vector2Int(0, -1); }
	static st_Vector2Int Left() { return st_Vector2Int(-1, 0); }
	static st_Vector2Int Right() { return st_Vector2Int(1, 0); }
	static st_Vector2Int Zero() { return st_Vector2Int(0, 0); }

	st_Vector2Int operator +(st_Vector2Int& Vector)
	{
		return st_Vector2Int(_X + Vector._X, _Y + Vector._Y);
	}

	st_Vector2Int operator -(st_Vector2Int& Vector)
	{
		return st_Vector2Int(_X - Vector._X, _Y - Vector._Y);
	}	

	st_Vector2Int operator *(int8 Value)
	{
		return st_Vector2Int(_X * Value, _Y * Value);
	}

	bool operator == (st_Vector2Int CellPosition)
	{
		if (_X == CellPosition._X && _Y == CellPosition._Y)
		{
			return true;
		}

		return false;
	}	

	int CellDistanceFromZero()
	{
		return abs(_X) + abs(_Y);
	}
	
	// 거리 구하기
	static int16 Distance(st_Vector2Int TargetCellPosition, st_Vector2Int MyCellPosition)
	{
		return (int16)sqrt(pow(TargetCellPosition._X - MyCellPosition._X, 2) + pow(TargetCellPosition._Y - MyCellPosition._Y, 2));
	}

	static en_MoveDir GetMoveDir(st_Vector2Int NormalVector)
	{
		if (NormalVector._X > 0)
		{
			return en_MoveDir::RIGHT;
		}

		if (NormalVector._X < 0)
		{
			return en_MoveDir::LEFT;
		}

		if (NormalVector._Y > 0)
		{
			return en_MoveDir::UP;
		}

		if (NormalVector._Y < 0)
		{
			return en_MoveDir::DOWN;
		}
	}	
};

struct st_PositionInt
{
	int32 _Y;
	int32 _X;

	st_PositionInt() {}

	st_PositionInt(int Y, int X)
	{
		_Y = Y;
		_X = X;
	}	

	bool operator ==(st_PositionInt& Position)
	{
		return (_Y == Position._Y) && (_X == Position._X);
	}

	bool operator !=(st_PositionInt& Position)
	{
		return !((*this) == Position);
	}

	bool operator <(const st_PositionInt& Position) const
	{
		return _X < Position._X || (_X == Position._X && _Y < Position._Y);
	}
};

struct st_AStarNodeInt
{
	int32 _F;
	int32 _G;
	st_PositionInt _Position;

	int32 _X;
	int32 _Y;
	
	st_AStarNodeInt() {}

	st_AStarNodeInt(int32 F, int32 G, int32 X, int32 Y)
	{
		_F = F;
		_G = G;
		_Position._X = X;
		_Position._Y = Y;		
	}	
};

class CMap
{
public:
	int32 _Left;
	int32 _Right;
	int32 _Up;
	int32 _Down;

	int32 _SizeX;
	int32 _SizeY;

	//-----------------------------------------
	// 맵 타일의 정보를 보관
	//-----------------------------------------
	en_TileMapEnvironment** _CollisionMapInfos;

	//-------------------------------------
	//맵 타일에 존재하는 게임 오브젝트 정보
	//-------------------------------------
	CGameObject*** _ObjectsInfos;

	//-----------------------------------------------------------
	// 맵 타일에 존재하는 아이템 정보
	// 한 타일에 존재 할 수 있는 아이템의 종류는 20개로 제한한다.
	//-----------------------------------------------------------
	CItem**** _Items;

	CMap(int MapId);

	//-------------------------------------------
	// 좌표 위치에 있는 오브젝트 반환
	//-------------------------------------------
	CGameObject* Find(st_Vector2Int& CellPosition);

	//-------------------------------------------
	// 좌표 위치에 있는 아이템들을 반환
	//-------------------------------------------
	CItem** FindItem(st_Vector2Int& ItemCellPosition);

	bool Cango(CGameObject* Object, float X, float Y);
	//----------------------------------------------------------------------------
	// 위치로 갈 수 있는지 확인
	// CheckObjects = 벽을 제외한 오브젝트를 충돌 대상으로 여길 것인지에 대한 판단
	// ( true : 해당위치에 오브젝트가 있는지 확인해서 있으면 충돌체로 판단한다. )
	//----------------------------------------------------------------------------
	bool CollisionCango(CGameObject* Object, st_Vector2Int& CellPosition, bool CheckObjects = true);

	//------------------------------------------------------------------------------------------------------------------------
	// 목적지 좌표값을 받아서 해당 좌표로 갈 수 있는지 없는지 판단
	// CheckObject = 벽을 제외한 해당 위치에 있는 오브젝트를 충돌 대상으로 여길 것인지에 대한 여부
	// ApplyCollision = 해당 함수를 호출 해준 오브젝트를 충돌 대상으로 여길 것인지에 대한 여부
	//------------------------------------------------------------------------------------------------------------------------
	bool ApplyMove(CGameObject* GameObject, st_Vector2Int& DestPosition, bool CheckObject = true, bool Applycollision = true);

	bool ApplyPositionUpdateItem(CItem* ItemObject, st_Vector2Int& NewPosition);

	//---------------------------------------
	// 맵에서 오브젝트 퇴장
	//---------------------------------------
	bool ApplyLeave(CGameObject* GameObject);

	bool ApplyPositionLeaveItem(CGameObject* GameObject);

	st_PositionInt CellToPositionInt(st_Vector2Int CellPosition);
	st_Vector2Int PositionToCellInt(st_PositionInt Position);

	vector<st_Vector2Int> FindPath(CGameObject* Object, st_Vector2Int StartCellPosition, st_Vector2Int DestCellPostion, bool CheckObjects = true, int32 MaxDistance = 10);
	vector<st_Vector2Int> CompletePath(map<st_PositionInt, st_PositionInt> Parents, st_PositionInt DestPosition);
};