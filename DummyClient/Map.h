#pragma once
#include "FileUtils.h"
#include "ClientInfo.h"

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

	//--------------------------------------------------------------
	// 위치값 정보를 받아서 방향값을 반환한다.
	//--------------------------------------------------------------
	static en_MoveDir GetDirectionFromVector(st_Vector2Int DirectionVector)
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
};

struct st_Position
{
	int32 _Y;
	int32 _X;

	st_Position() {}

	st_Position(int Y, int X)
	{
		_Y = Y;
		_X = X;
	}

	bool operator ==(st_Position& Position)
	{
		return (_Y == Position._Y) && (_X == Position._X);
	}

	bool operator !=(st_Position& Position)
	{
		return !((*this) == Position);
	}

	bool operator <(const st_Position& Position) const
	{
		return _X < Position._X || (_X == Position._X && _Y < Position._Y);
	}
};

struct st_AStarNode
{
	int32 _F;
	int32 _G;
	st_Position _Position;

	int32 _X;
	int32 _Y;

	st_AStarNode() {}

	st_AStarNode(int32 F, int32 G, int32 X, int32 Y)
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
	// 맵 환경 오브젝트의 정보를 보관
	//-----------------------------------------
	en_MapObjectInfo** _CollisionMapInfos;

	//-------------------------------------
	//맵 타일에 존재하는 게임 오브젝트 정보
	//-------------------------------------
	st_Client*** _ObjectsInfos;

	//-----------------------------------------------------------
	// 맵 타일에 존재하는 아이템 정보
	// 한 타일에 존재 할 수 있는 아이템의 종류는 20개로 제한한다.
	//-----------------------------------------------------------
	CMap(int MapId);
	
	//-------------------------------------------
	// 좌표 위치에 있는 오브젝트 반환
	//-------------------------------------------
	st_Client* Find(st_Vector2Int& CellPosition);	

	bool Cango(st_Client* Object, float X, float Y);
	//----------------------------------------------------------------------------
	// 위치로 갈 수 있는지 확인
	// CheckObjects = 벽을 제외한 오브젝트를 충돌 대상으로 여길 것인지에 대한 판단
	// ( true : 해당위치에 오브젝트가 있는지 확인해서 있으면 충돌체로 판단한다. )
	//----------------------------------------------------------------------------
	bool CollisionCango(st_Client* Object, st_Vector2Int& CellPosition, bool CheckObjects = true);

	//------------------------------------------------------------------------------------------------------------------------
	// 목적지 좌표값을 받아서 해당 좌표로 갈 수 있는지 없는지 판단
	// CheckObject = 벽을 제외한 해당 위치에 있는 오브젝트를 충돌 대상으로 여길 것인지에 대한 여부
	// ApplyCollision = 해당 함수를 호출 해준 오브젝트를 충돌 대상으로 여길 것인지에 대한 여부
	//------------------------------------------------------------------------------------------------------------------------
	bool ApplyMove(st_Client* GameObject, st_Vector2Int& DestPosition, bool CheckObject = true, bool Applycollision = true);	

	//---------------------------------------
	// 맵에서 오브젝트 퇴장
	//---------------------------------------
	bool ApplyLeave(st_Client* GameObject);	

	st_Position CellToPosition(st_Vector2Int CellPosition);
	st_Vector2Int PositionToCell(st_Position Position);
	
};