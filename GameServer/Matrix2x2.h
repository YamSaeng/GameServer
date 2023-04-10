#pragma once
#include <array>
#include "Vector2.h"

struct Matrix2x2
{
public:	
	Matrix2x2()
	{
		Cols[0] = Vector2::UnitX;
		Cols[1] = Vector2::UnitY;
	}

	Matrix2x2(Vector2 Col0, Vector2 Col1)
	{
		Cols[0] = Col0;
		Cols[1] = Col1;
	}

	array<Vector2, 2> Cols;

	FORCEINLINE Vector2& operator[](BYTE Index)
	{
		return Cols[Index];
	}

	FORCEINLINE Vector2 operator*(Vector2& Vector) const
	{
		Matrix2x2 transposedMatrix = Transpose();
		return Vector2(transposedMatrix[0].Dot(Vector),transposedMatrix[1].Dot(Vector));
	}

	// 전치 연산
	FORCEINLINE Matrix2x2 Transpose() const
	{
		return Matrix2x2(Vector2(Cols[0].X, Cols[1].X),Vector2(Cols[0].Y, Cols[1].Y));
	}	
};

