#pragma once
#include <xmmintrin.h>
#include "Math.h"

struct Vector2
{
	static Vector2 UnitX;
	static Vector2 UnitY;
	static Vector2 Up;
	static Vector2 Down;
	static Vector2 Left;
	static Vector2 Right;
	static Vector2 Zero;
	static Vector2 One;

	float X;
	float Y;

	Vector2() { X = 0; Y = 0; }
	Vector2(float x, float y) { X = x; Y = y; }

	Vector2 operator-() const
	{
		return Vector2(-X, -Y);
	}

	Vector2 operator+(Vector2& Vector) const
	{
		return Vector2(X + Vector.X, Y + Vector.Y);
	}

	Vector2 operator-(Vector2& Vector) const
	{
		return Vector2(X - Vector.X, Y - Vector.Y);
	}

	Vector2 operator*(float Sclar) const
	{
		return Vector2(X * Sclar, Y * Sclar);
	}

	Vector2 operator/(float Sclar) const
	{
		return Vector2(X / Sclar, Y / Sclar);
	}

	Vector2 operator+=(Vector2 Vector)
	{
		X += Vector.X;
		Y += Vector.Y;

		return *this;
	}

	Vector2 operator-=(Vector2 Vector)
	{
		X -= Vector.X;
		Y -= Vector.Y;

		return *this;
	}

	Vector2 operator*=(Vector2 Vector)
	{
		X *= Vector.X;
		Y *= Vector.Y;

		return *this;
	}

	Vector2 operator/=(Vector2 Vector)
	{
		X /= Vector.X;
		Y /= Vector.Y;

		return *this;
	}

	float SizeSquared()
	{
		return X * X + Y * Y;
	}

	Vector2 Normalize()
	{
		float SquaredSum = SizeSquared();

		if (SquaredSum != 0)
		{
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

			return Vector2(X, Y) * temp;
		}
		else
		{
			return Vector2::Zero;
		}
	}

	float Dot(Vector2 Vector)
	{
		return X * Vector.X + Y * Vector.Y;
	}

	float Size(Vector2 Vector)
	{
		return sqrt(SizeSquared());
	}
	
	static float Distance(Vector2 TargetCellPosition, Vector2 MyCellPosition)
	{
		return sqrt(((TargetCellPosition.X - MyCellPosition.X) * (TargetCellPosition.X - MyCellPosition.X))
			+ ((TargetCellPosition.Y - MyCellPosition.Y) * (TargetCellPosition.Y - MyCellPosition.Y)));
	}

	// 라디안
	float Angle() const
	{
		return atan2f(Y, X);
	}

	// 라디안을 각도로
	float AngleToDegree() const
	{
		return Math::RadianToDegree(atan2f(Y, X));
	}

	// 두 벡터사이 각도
	static float AngleBetweenVector(Vector2 TargetVector, Vector2 MyVector)
	{
		Vector2 TargetVectorNormal = TargetVector.Normalize();
		Vector2 MyVectorNormal = MyVector.Normalize();

		float Dot = TargetVectorNormal.Dot(MyVectorNormal);
		float ACosf = acosf(Dot);
		float Angle = ACosf * 180.0f / Math::PI;

		return Angle;
	}

	// 목표물 위치, 내 위치, 내가 바라보는 방향값, 시야각 크기
	// 목표물이 내 시야각 안에 있는지 확인
	static bool CheckFieldOfView(Vector2 TargetPosition, Vector2 CheckPosition,
		Vector2 DirectionVector, float Angle, float Distance)
	{
		float TargetDistance = Vector2::Distance(TargetPosition, CheckPosition);

		// 대상 방향 벡터 구하기
		Vector2 TargetDirVector = TargetPosition - CheckPosition;
		// 방향 벡터 정규화
		Vector2 NormalizeDirVector = TargetDirVector.Normalize();

		// 입력받은 시야각의 절반값에 대한 cos 값을 구함
		float FovCos = cosf((Angle * 0.5f) * Math::PI / 180.0f);

		if (Distance >= TargetDistance)
		{
			if (NormalizeDirVector.Dot(DirectionVector) >= FovCos)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
};

struct Vector2Int
{
	static Vector2Int UnitX;
	static Vector2Int UnitY;
	static Vector2Int Up;
	static Vector2Int Down;
	static Vector2Int Left;
	static Vector2Int Right;
	static Vector2Int Zero;
	static Vector2Int One;

	int32 X;
	int32 Y;

	Vector2Int() { X = 0; Y = 0; };
	Vector2Int(int32 x, int32 y) { X = x; Y = y; }

	Vector2Int operator +(Vector2Int& Vector)
	{
		return Vector2Int(X + Vector.X, Y + Vector.Y);
	}

	Vector2Int operator -(Vector2Int& Vector)
	{
		return Vector2Int(X - Vector.X, Y - Vector.Y);
	}

	Vector2Int operator *(int16 Value)
	{
		return Vector2Int(X * Value, Y * Value);
	}

	bool operator == (Vector2Int CellPosition)
	{
		return (X == CellPosition.X) && (Y == CellPosition.Y);
	}

	bool operator != (Vector2Int CellPosition)
	{
		return !((*this) == CellPosition);
	}

	bool operator <(const Vector2Int& CellPosition) const
	{
		return X < CellPosition.X || (X == CellPosition.X && Y < CellPosition.Y);
	}	

	// 거리 구하기
	static int16 Distance(Vector2Int TargetCellPosition, Vector2Int MyCellPosition)
	{
		return (int16)sqrt(pow(TargetCellPosition.X - MyCellPosition.X, 2) + pow(TargetCellPosition.Y - MyCellPosition.Y, 2));
	}

	Vector2Int Direction()
	{
		Vector2Int Dir;

		if (X > 0)
		{
			Dir.X = 1;
		}
		else if (X < 0)
		{
			Dir.X = -1;
		}
		else
		{
			Dir.X = 0;
		}

		if (Y > 0)
		{
			Dir.Y = 1;
		}
		else if (Y < 0)
		{
			Dir.Y = -1;
		}
		else
		{
			Dir.Y = 0;
		}

		return Dir;
	}
};