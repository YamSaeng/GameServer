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
};

