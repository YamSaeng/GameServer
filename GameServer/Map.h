#pragma once
#include "FileUtils.h"
#include "GameObjectInfo.h"
#include <xmmintrin.h>

class CItem;
class CGameObject;
class CSector;
class CPlayer;
class CMonster;
class CChannelManager;

struct st_Vector2
{
	static constexpr float PI = { 3.14159265358979323846f };

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

	st_Vector2 operator * (float& Sclar)
	{
		return st_Vector2(_X * Sclar, _Y * Sclar);
	}

	// �Ÿ� ���ϱ�
	static float Distance(st_Vector2 TargetCellPosition, st_Vector2 MyCellPosition)
	{
		return (float)sqrt(pow(TargetCellPosition._X - MyCellPosition._X, 2) + pow(TargetCellPosition._Y - MyCellPosition._Y, 2));
	}

	// ���� ũ�� ���ϱ�
	float Size(st_Vector2 Vector)
	{
		return sqrt(SizeSquared());
	}

	float SizeSquared()
	{
		return _X * _X + _Y * _Y;
	}

	// ���� ����ȭ
	st_Vector2 Normalize()
	{		
		float SquaredSum = SizeSquared();
		
		const __m128 fOneHalf = _mm_set_ss(0.5f);
		__m128 Y0, X0, X1, X2, FOver2;
		float temp;

		Y0 = _mm_set_ss(SquaredSum);
		X0 = _mm_rsqrt_ss(Y0);	// 1/sqrt estimate (12 bits)
		FOver2 = _mm_mul_ss(Y0, fOneHalf);

		// 1st Newton-Raphson iteration
		X1 = _mm_mul_ss(X0, X0);
		X1 = _mm_sub_ss(fOneHalf, _mm_mul_ss(FOver2, X1));
		X1 = _mm_add_ss(X0, _mm_mul_ss(X0, X1));

		// 2nd Newton-Raphson iteration
		X2 = _mm_mul_ss(X1, X1);
		X2 = _mm_sub_ss(fOneHalf, _mm_mul_ss(FOver2, X2));
		X2 = _mm_add_ss(X1, _mm_mul_ss(X1, X2));

		_mm_store_ss(&temp, X2);

		return st_Vector2(_X, _Y) * temp;		
	}

	static en_MoveDir GetMoveDir(st_Vector2 NormalVector)
	{
		NormalVector._X = round(NormalVector._X);
		NormalVector._Y = round(NormalVector._Y);

		if (NormalVector._X < 0)
		{
			return en_MoveDir::LEFT;
		}

		if (NormalVector._X > 0)
		{
			return en_MoveDir::RIGHT;
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

	float Dot(st_Vector2 InVector)
	{
		return _X * InVector._X + _Y * InVector._Y;
	}

	static bool CheckFieldOfView(st_Vector2 TargetPosition, st_Vector2 CheckPosition, en_MoveDir Dir, int16 Angle)
	{
		// ��� ���� ���� ���ϱ�
		st_Vector2 TargetDirVector = TargetPosition - CheckPosition;
		// ���� ���� ����ȭ
		st_Vector2 NormalizeDirVector = TargetDirVector.Normalize();

		// 
		float FovCos = cosf((Angle * 0.5f) * PI / 180.0f);

		st_Vector2 ForwardVector;

		switch (Dir)
		{
		case en_MoveDir::UP:
			ForwardVector._X = 0;
			ForwardVector._Y = 1.0f;
			break;
		case en_MoveDir::DOWN:
			ForwardVector._X = 0;
			ForwardVector._Y = -1.0f;
			break;
		case en_MoveDir::LEFT:
			ForwardVector._X = -1.0f;
			ForwardVector._Y = 0;
			break;
		case en_MoveDir::RIGHT:
			ForwardVector._X = 1.0f;
			ForwardVector._Y = 0;
			break;		
		}

		if (NormalizeDirVector.Dot(ForwardVector) >= FovCos)
		{
			return true;
		}
		else
		{
			return false;
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

	// ���� ����
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

	// �Ÿ� ���ϱ�
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
	int64 _MapID;

	wstring _MapName;

	int32 _Left;
	int32 _Right;
	int32 _Up;
	int32 _Down;

	int32 _SizeX;
	int32 _SizeY;

	//-----------------------------------------
	// �� Ÿ���� ������ ����
	//-----------------------------------------
	en_TileMapEnvironment** _CollisionMapInfos;

	//-------------------------------------
	//�� Ÿ�Ͽ� �����ϴ� ���� ������Ʈ ����
	//-------------------------------------
	CGameObject*** _ObjectsInfos;

	//-----------------------------------------------------------
	// �� Ÿ�Ͽ� �����ϴ� ������ ����
	// �� Ÿ�Ͽ� ���� �� �� �ִ� �������� ������ 20���� �����Ѵ�.
	//-----------------------------------------------------------
	CItem**** _Items;

	CMap();
	~CMap();

	void MapInit(int64 MapID, wstring MapName, int32 SectorSize, int8 ChannelCount);

	CSector* GetSector(st_Vector2Int CellPosition);
	CSector* GetSector(int32 IndexY, int32 IndexX);

	//-----------------------------------------------------------------
	// �� ���� ���� ��ȯ
	//-----------------------------------------------------------------
	vector<CSector*> GetAroundSectors(st_Vector2Int CellPosition, int32 Range);	
	//------------------------------------------------------------------------------------------------------
	// ������Ʈ �þ� ���� �ȿ� �ִ� ������Ʈ ���̵� ��� ��ȯ
	//------------------------------------------------------------------------------------------------------
	vector<st_FieldOfViewInfo> GetFieldOfViewObjects(CGameObject* Object, int16 Range, bool ExceptMe = true);
	//---------------------------------------------------------------------------------
	// ������Ʈ �þ� ���� �ȿ� �ִ� �÷��̾� ���̵� ��� ��ȯ
	//---------------------------------------------------------------------------------
	vector<st_FieldOfViewInfo> GetFieldOfViewPlayers(CGameObject* Object, int16 Range, bool ExceptMe = true);
	//----------------------------------------------------------------------------------------
	// ������Ʈ ���� ���� ��� ��ȯ
	//----------------------------------------------------------------------------------------
	vector<CMonster*> GetAroundMonster(CGameObject* Object, int16 Range, bool ExceptMe = true);	
	//--------------------------------------------------------------------------------------
	// �� �þ� ���� �÷��̾� ��ȯ
	//--------------------------------------------------------------------------------------
	vector<CPlayer*> GetFieldOfViewPlayer(CGameObject* Object, int16 Range, bool ExceptMe = true);

	//-------------------------------------------------------
	// �� ��ó �÷��̾� ��ȯ
	//-------------------------------------------------------
	CGameObject* FindNearPlayer(CGameObject* Object, int32 Range, bool* CollisionCango);

	//-------------------------------------------
	// ��ǥ ��ġ�� �ִ� ������Ʈ ��ȯ
	//-------------------------------------------
	CGameObject* Find(st_Vector2Int& CellPosition);

	//-------------------------------------------
	// ��ǥ ��ġ�� �ִ� �����۵��� ��ȯ
	//-------------------------------------------
	CItem** FindItem(st_Vector2Int& ItemCellPosition);

	bool Cango(CGameObject* Object, float X, float Y);
	//----------------------------------------------------------------------------
	// ��ġ�� �� �� �ִ��� Ȯ��
	// CheckObjects = ���� ������ ������Ʈ�� �浹 ������� ���� �������� ���� �Ǵ�
	// ( true : �ش���ġ�� ������Ʈ�� �ִ��� Ȯ���ؼ� ������ �浹ü�� �Ǵ��Ѵ�. )
	//----------------------------------------------------------------------------
	bool CollisionCango(CGameObject* Object, st_Vector2Int& CellPosition, bool CheckObjects = true);

	//------------------------------------------------------------------------------------------------------------------------
	// ������ ��ǥ���� �޾Ƽ� �ش� ��ǥ�� �� �� �ִ��� ������ �Ǵ�
	// CheckObject = ���� ������ �ش� ��ġ�� �ִ� ������Ʈ�� �浹 ������� ���� �������� ���� ����
	// ApplyCollision = �ش� �Լ��� ȣ�� ���� ������Ʈ�� �浹 ������� ���� �������� ���� ����
	//------------------------------------------------------------------------------------------------------------------------
	bool ApplyMove(CGameObject* GameObject, st_Vector2Int& DestPosition, bool CheckObject = true, bool Applycollision = true);

	bool ApplyPositionUpdateItem(CItem* ItemObject, st_Vector2Int& NewPosition);

	//---------------------------------------
	// �ʿ��� ������Ʈ ����
	//---------------------------------------
	bool ApplyLeave(CGameObject* GameObject);

	bool ApplyPositionLeaveItem(CGameObject* GameObject);

	st_PositionInt CellToPositionInt(st_Vector2Int CellPosition);
	st_Vector2Int PositionToCellInt(st_PositionInt Position);

	vector<st_Vector2Int> FindPath(CGameObject* Object, st_Vector2Int StartCellPosition, st_Vector2Int DestCellPostion, bool CheckObjects = true, int32 MaxDistance = 10);
	vector<st_Vector2Int> CompletePath(map<st_PositionInt, st_PositionInt> Parents, st_PositionInt DestPosition);

	CChannelManager* GetChannelManager();
private:
	CChannelManager* _ChannelManager;
	CSector** _Sectors;
	int32 _SectorSize;

	int32 _SectorCountX;
	int32 _SectorCountY;
};