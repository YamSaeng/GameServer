#pragma once
#include "Data.h"
#include "FileUtils.h"
#undef min
#undef max
#include "rapidjson/document.h"

class CDataManager
{
public:
	map<int32, st_ItemData*> _Items;
	map<int16, st_PlayerStatusData*> _Status;
	map<int32, st_MonsterData*> _Monsters;
	map<int32, st_SkillData*> _Skills;
	map<int32, st_EnvironmentData*> _Environments;

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
};

