#pragma once

class CGameObject;

struct st_Vector2Int
{
	int32 _X;
	int32 _Y;

	st_Vector2Int(int X, int Y)
	{
		_X = X;
		_Y = Y;
	}	

	// πÊ«‚ ∫§≈Õ
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
	
	bool* _CollisionMapInfo;	
	CGameObject* _ObjectsInfo;

	CMap(int MapId);
	
};

