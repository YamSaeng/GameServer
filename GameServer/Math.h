#pragma once
#include <atlbase.h>
#include "GameObjectInfo.h"

class CMath
{
public:
	static int32 CalculateMeleeDamage(bool* InOutCritical, int32 TargetDefence, int32 MinDamage, int32 MaxDamage, int16 CriticalPoint)
	{
		random_device Seed;
		default_random_engine Eng(Seed());		

		mt19937 Gen(Seed());
		uniform_int_distribution<int> DamageChoiceRandom(MinDamage, MaxDamage);

		int32 ChoiceRandomDamage = DamageChoiceRandom(Gen);

		int32 CriticalDamage = 0;

		if (*InOutCritical == true)
		{
			// 크리티컬 판단
			float CriticalPointCheck = CriticalPoint / 1000.0f;
			bernoulli_distribution CriticalCheck(CriticalPointCheck);
			bool IsCritical = CriticalCheck(Eng);

			*InOutCritical = IsCritical;

			CriticalDamage = IsCritical ? ChoiceRandomDamage * 2 : ChoiceRandomDamage;
		}				
		else
		{
			CriticalDamage = ChoiceRandomDamage;
		}		

		float DefenceRate = (float)pow(((float)(200 - TargetDefence)) / 20, 2) * 0.01f;

		int32 FinalDamage = (int32)(CriticalDamage * DefenceRate);

		return FinalDamage;
	}	
};

