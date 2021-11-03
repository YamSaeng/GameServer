#pragma once
#include "Data.h"
#include "FileUtils.h"
#undef min
#undef max
#include "rapidjson/document.h"

class CDataManager
{
public:
	map<int16, st_ItemData*> _Items;
	map<int16, st_ConsumableData*> _Consumables;
	map<int32, st_ObjectStatusData*> _WarriorStatus;
	map<int32, st_ObjectStatusData*> _ShamanStatus;
	map<int32, st_MonsterData*> _Monsters;

	//------------------------------------------------------
	// 스킬 데이터		
	//------------------------------------------------------
	// 공용 스킬 데이터
	//------------------------------------------------------
	map<int16, st_AttackSkillData*> _PublicAttackSkillDatas;
	map<int16, st_HealSkillData*> _PublicHealSkillDatas;
	map<int16, st_BufSkillData*> _PublicBufSkillDatas;
	//------------------------------------------------------
	// 전사 스킬 데이터
	//------------------------------------------------------
	map<int16, st_AttackSkillData*> _WarriorAttackSkillDatas;
	map<int16, st_HealSkillData*> _WarriorHealSkillDatas;
	map<int16, st_BufSkillData*> _WarriorBufSkillDatas;
	//------------------------------------------------------
	// 주술사 스킬 데이터
	//------------------------------------------------------
	map<int16, st_AttackSkillData*> _ShamanAttackSkillDatas;
	map<int16, st_HealSkillData*> _ShamanHealSkillDatas;
	map<int16, st_BufSkillData*> _ShamanBufSkillDatas;
	//------------------------------------------------------
	// 도사 스킬 데이터
	//------------------------------------------------------
	map<int16, st_AttackSkillData*> _TaioistAttackSkillDatas;
	map<int16, st_HealSkillData*> _TaioistHealSkillDatas;
	map<int16, st_BufSkillData*> _TaioistBufSkillDatas;

	map<int32, st_EnvironmentData*> _Environments;
	map<int8, st_CraftingItemCategoryData*> _CraftingData;
	map<int32, st_LevelData*> _LevelDatas;

	CDataManager()
	{

	}

	~CDataManager()
	{
		
	}

	void LoadDataItem(wstring LoadFileName);
	void LoadDataPlayerCharacterStatus(wstring LoadFileName);	
	void LoadDataLevel(wstring LoadFileName);
	void LoadDataMonster(wstring LoadFileName);
	void LoadDataPublicSkill(wstring LoadFileName);
	void LoadDataWarriorSkill(wstring LoadFileName);
	void LoadDataShamanSkill(wstring LoadFileName);
	void LoadDataTaioistSkill(wstring LoadFileName);
	void LoadDataEnvironment(wstring LoadFileName);
	void LoadDataCrafting(wstring LoadFileName);

	st_SkillData* FindSkillData(en_SkillMediumCategory FindSkillMediumCategory, en_SkillType FindSkillType);	
};