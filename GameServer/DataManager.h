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
	map<int16, st_PlayerStatusData*> _Status;
	map<int32, st_MonsterData*> _Monsters;
	map<int16, st_SkillData*> _PlayerMeleeSkills;
	map<int16, st_SkillData*> _PlayerMagicSkills;
	map<int16, st_SkillData*> _MonsterMeleeSkills;
	map<int16, st_SkillData*> _MonsterMagicSkills;
	map<int32, st_EnvironmentData*> _Environments;
	map<int8, st_CraftingItemCategoryData*> _CraftingData;

	CDataManager()
	{

	}

	~CDataManager()
	{
		
	}

	void LoadDataItem(wstring LoadFileName);
	void LoadDataStatus(wstring LoadFileName);	
	void LoadDataMonster(wstring LoadFileName);
	void LoadDataSkill(wstring LoadFileName);
	void LoadDataEnvironment(wstring LoadFileName);
	void LoadDataCrafting(wstring LoadFileName);
};

