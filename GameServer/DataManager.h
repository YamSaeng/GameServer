#pragma once
#include "Data.h"
#include "FileUtils.h"
#undef min
#undef max
#include "rapidjson/document.h"

class CDataManager
{
public:
	map<int16, st_ItemInfo*> _Items;	

	map<int32, st_ObjectStatusData*> _WarriorStatus;
	map<int32, st_ObjectStatusData*> _ShamanStatus;
	map<int32, st_ObjectStatusData*> _TaioistStatus;
	map<int32, st_ObjectStatusData*> _ThiefStatus;
	map<int32, st_ObjectStatusData*> _ArcherStatus;

	//------------------------------------------------------
	// 몬스터 데이터
	//------------------------------------------------------
	map<en_GameObjectType, st_MonsterData*> _Monsters;

	st_MonsterAggroData _MonsterAggroData;

	//------------------------------------------------------
	// 스킬 데이터		
	//------------------------------------------------------
	// 공용 스킬 데이터
	//------------------------------------------------------
	map<int16, st_AttackSkillInfo*> _PublicAttackSkillDatas;
	map<int16, st_TacTicSkillInfo*> _PublicTacTicSkillDatas;
	map<int16, st_BufSkillInfo*> _PublicBufSkillDatas;
	//------------------------------------------------------
	// 전사 스킬 데이터
	//------------------------------------------------------
	map<int16, st_AttackSkillInfo*> _WarriorAttackSkillDatas;
	map<int16, st_TacTicSkillInfo*> _WarriorTacTicSkillDatas;
	map<int16, st_BufSkillInfo*> _WarriorBufSkillDatas;
	//------------------------------------------------------
	// 주술사 스킬 데이터
	//------------------------------------------------------
	map<int16, st_AttackSkillInfo*> _ShamanAttackSkillDatas;
	map<int16, st_TacTicSkillInfo*> _ShamanTacTicSkillDatas;
	map<int16, st_BufSkillInfo*> _ShamanBufSkillDatas;
	//------------------------------------------------------
	// 도사 스킬 데이터
	//------------------------------------------------------
	map<int16, st_AttackSkillInfo*> _TaioistAttackSkillDatas;
	map<int16, st_TacTicSkillInfo*> _TaioistTacTicSkillDatas;
	map<int16, st_BufSkillInfo*> _TaioistBufSkillDatas;
	//------------------------------------------------------
	// 도적 스킬 데이터
	//------------------------------------------------------
	map<int16, st_AttackSkillInfo*> _ThiefAttackSkillDatas;
	map<int16, st_TacTicSkillInfo*> _ThiefTacTicSkillDatas;
	map<int16, st_BufSkillInfo*> _ThiefBufSkillDatas;
	//------------------------------------------------------
	// 궁사 스킬 데이터
	//------------------------------------------------------
	map<int16, st_AttackSkillInfo*> _ArcherAttackSkillDatas;
	map<int16, st_TacTicSkillInfo*> _ArcherTacTicSkillDatas;
	map<int16, st_BufSkillInfo*> _ArcherBufSkillDatas;

	//------------------------------------------------------
	// 환경 데이터
	//------------------------------------------------------
	map<en_GameObjectType, st_EnvironmentData*> _Environments;
	//------------------------------------------------------
	// 작물 데이터
	//------------------------------------------------------
	map<en_GameObjectType, st_CropData*> _Crops;

	map<int16, st_CraftingTableRecipe*> _CraftingTableData;
	map<int8,  st_CraftingItemCategory*> _CraftingData;
	map<int32, st_LevelData*> _LevelDatas;

	map<int64, st_MapInfoData*> _MapInfoDatas;

	map<int8, st_OptionItemInfo*> _OptionItemInfoDatas;

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
	void LoadDataMonsterAggro(wstring LoadFileName);
	void LoadDataPublicSkill(wstring LoadFileName);
	void LoadDataWarriorSkill(wstring LoadFileName);
	void LoadDataShamanSkill(wstring LoadFileName);
	void LoadDataTaioistSkill(wstring LoadFileName);
	void LoadDataThiefSkill(wstring LoadFileName);
	void LoadDataArcherSkill(wstring LoadFileName);
	void LoadDataEnvironment(wstring LoadFileName);
	void LoadDataCrop(wstring LoadFileName);
	void LoadDataCrafting(wstring LoadFileName);
	void LoadDataCraftingTable(wstring LoadFileName);
	void LoadDataMapInfo(wstring LoadFileName);
	void LoadDataOptionInfo(wstring LoadFileName);

	st_SkillInfo* FindSkillData(en_SkillMediumCategory FindSkillMediumCategory, en_SkillType FindSkillType);
	st_ObjectStatusData* FindObjectStatusData(en_GameObjectType GameObjectType, int16 Level);
	st_ItemInfo* FindItemData(en_SmallItemCategory FindItemCategory);	
};