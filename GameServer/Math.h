#pragma once
#include <atlbase.h>

struct Math
{
public:

	static constexpr float PI = { 3.14159265358979323846f };
	static constexpr float InvPI = { 0.31830988618f };
	static constexpr float HalfPI = { 1.57079632679f };		

	FORCEINLINE static int32 RandomNumberInt(int32 MinNumber, int32 MaxNumber)
	{
		random_device RandomSeed;
		mt19937 Gen(RandomSeed());

		uniform_int_distribution<int> RandomNumberCreate(MinNumber, MaxNumber);
		
		int32 RandomNumber = RandomNumberCreate(Gen);

		return RandomNumber;
	}

	FORCEINLINE static float RandomNumberFloat(float MinNumber, float MaxNumber)
	{
		random_device RandomSeed;
		mt19937 Gen(RandomSeed());

		uniform_real_distribution<float> RandomNumberCreate(MinNumber, MaxNumber);

		float RandomNumber = RandomNumberCreate(Gen);

		return RandomNumber;
	}

	FORCEINLINE static bool IsSuccess(float Probability)
	{
		random_device RandomSeed;
		mt19937 Gen(RandomSeed());

		bernoulli_distribution Check(Probability);

		return Check(Gen);
	}

	// 각도 -> 라디안
	FORCEINLINE static float DegreeToRadian(float Degree)
	{
		return Degree * PI / 180.0f;
	}

	// 라디안 -> 각도
	FORCEINLINE static float RadianToDegree(float Radian)
	{
		return Radian * 180.0f * InvPI;
	}
	
	// Sin Cos 얻기 ( 라디안 값 )
	FORCEINLINE static void GetSinCosRadian(float& OutSin, float& OutCos, float InRadian)
	{
		// Copied from UE4 Source Code
		// Map Value to y in [-pi,pi], x = 2*pi*quotient + remainder.
		float quotient = (InvPI * 0.5f) * InRadian;
		if (InRadian >= 0.0f)
		{
			quotient = (float)((int)(quotient + 0.5f));
		}
		else
		{
			quotient = (float)((int)(quotient - 0.5f));
		}
		float y = InRadian - (2.0f * PI) * quotient;

		// Map y to [-pi/2,pi/2] with sin(y) = sin(Value).
		float sign = 0.f;
		if (y > HalfPI)
		{
			y = PI - y;
			sign = -1.0f;
		}
		else if (y < -HalfPI)
		{
			y = -PI - y;
			sign = -1.0f;
		}
		else
		{
			sign = +1.0f;
		}

		float y2 = y * y;

		// 11-degree minimax approximation
		OutSin = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;

		// 10-degree minimax approximation
		float p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
		OutCos = sign * p;
	}

	// SinCos 얻기 ( 각도 값 )
	FORCEINLINE static void GetSinCosDegree(OUT float& Sin, OUT float& Cos, IN float Degree)
	{
		if (Degree == 0.f)
		{
			Sin = 0.f;
			Cos = 1.f;
		}
		else if (Degree == 90.f)
		{
			Sin = 1.f;
			Cos = 0.f;
		}
		else if (Degree == 180.f)
		{
			Sin = 0.f;
			Cos = -1.f;
		}
		else if (Degree == 270.f)
		{
			Sin = -1.f;
			Cos = 0.f;
		}
		else
		{
			GetSinCosRadian(Sin, Cos, Math::DegreeToRadian(Degree));
		}
	}	
};

