#pragma once
#include <atlbase.h>

class CMath
{
public:
	static int32 CalculateMeleeDamage(int32 TargetDefence, int32 MinDamage, int32 MaxDamage, int16 CriticalPoint, bool *OutCritical)
	{
		random_device Seed;
		default_random_engine Eng(Seed());		

		// 크리티컬 판단
		float CriticalPointCheck = CriticalPoint / 1000.0f;
		bernoulli_distribution CriticalCheck(CriticalPointCheck);
		bool IsCritical = CriticalCheck(Eng);

		*OutCritical = IsCritical;

		mt19937 Gen(Seed());
		uniform_int_distribution<int> DamageChoiceRandom( MinDamage, MaxDamage );

		int32 ChoiceRandomDamage = DamageChoiceRandom(Gen);

		int32 CriticalDamage = IsCritical ? ChoiceRandomDamage * 2 : ChoiceRandomDamage;

		float DefenceRate = (float)pow(((float)(200 - TargetDefence)) / 20, 2) * 0.01f;

		int32 FinalDamage = (int32)(CriticalDamage * DefenceRate);

		return FinalDamage;
	}
};

