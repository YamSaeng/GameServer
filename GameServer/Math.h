#pragma once
#include <atlbase.h>
#include "GameObjectInfo.h"

class CMath
{
public:
	static int32 CalculateDamage(en_SkillType SkillType,
		int32& Str, int32& Dex, int32& Int, int32& Luck, 
		bool* InOutCritical,
		int32 TargetDefence, 
		int32 MinDamage, int32 MaxDamage, 
		int16 CriticalPoint)
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

		int32 FinalDamage = 0;

		switch (SkillType)
		{				
		case en_SkillType::SKILL_DEFAULT_ATTACK:
		case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FIERCE_ATTACK:
		case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK:
		case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_SHAHONE:
		case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CHOHONE:
		case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_SMASH:
		case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_QUICK_CUT:
		case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_FAST_CUT:
		case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_ATTACK:
		case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_STEP:
		case en_SkillType::SKILL_ASSASSINATION_ACTIVE_BUF_WEAPON_POISON:
		case en_SkillType::SKILL_GOBLIN_ACTIVE_MELEE_DEFAULT_ATTACK:

			FinalDamage = (CriticalDamage + Str / 2) * (1 - ((float)TargetDefence / (100.0f + (float)TargetDefence)));

			break;
		case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_FLAME_HARPOON:
		case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_CHAIN:
		case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE:
		case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_LIGHTNING_STRIKE:
		case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_HEL_FIRE:
		case en_SkillType::SKILL_SPELL_ACTIVE_BUF_TELEPORT:
		case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_DIVINE_STRIKE:

			FinalDamage = (CriticalDamage + Int / 2) * (1 - ((float)TargetDefence / (100.0f + (float)TargetDefence)));

			break;		
		case en_SkillType::SKILL_SHOOTING_ACTIVE_ATTACK_SNIFING:
			break;				
		}		

		/*float DefenceRate = (float)pow(((float)(200 - 1)) / 20, 2) * 0.01f;
		int32 FinalDamage = (int32)(CriticalDamage * DefenceRate);*/

		return FinalDamage;
	}	
};

