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
	map<int32, st_StatusData*> _Status;
	map<int32, st_MonsterData*> _Monsters;
	map<int32, st_SkillData*> _Skills;

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
};

