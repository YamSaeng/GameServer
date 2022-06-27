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
	map<int16, st_AttackSkillData*> _PublicAttackSkillDatas;
	map<int16, st_TacTicSkillData*> _PublicTacTicSkillDatas;
	map<int16, st_BufSkillData*> _PublicBufSkillDatas;
	//------------------------------------------------------
	// 전사 스킬 데이터
	//------------------------------------------------------
	map<int16, st_AttackSkillData*> _WarriorAttackSkillDatas;
	map<int16, st_TacTicSkillData*> _WarriorTacTicSkillDatas;
	map<int16, st_BufSkillData*> _WarriorBufSkillDatas;
	//------------------------------------------------------
	// 주술사 스킬 데이터
	//------------------------------------------------------
	map<int16, st_AttackSkillData*> _ShamanAttackSkillDatas;
	map<int16, st_TacTicSkillData*> _ShamanTacTicSkillDatas;
	map<int16, st_BufSkillData*> _ShamanBufSkillDatas;
	//------------------------------------------------------
	// 도사 스킬 데이터
	//------------------------------------------------------
	map<int16, st_AttackSkillData*> _TaioistAttackSkillDatas;
	map<int16, st_TacTicSkillData*> _TaioistTacTicSkillDatas;
	map<int16, st_BufSkillData*> _TaioistBufSkillDatas;
	//------------------------------------------------------
	// 도적 스킬 데이터
	//------------------------------------------------------
	map<int16, st_AttackSkillData*> _ThiefAttackSkillDatas;
	map<int16, st_TacTicSkillData*> _ThiefTacTicSkillDatas;
	map<int16, st_BufSkillData*> _ThiefBufSkillDatas;
	//------------------------------------------------------
	// 궁사 스킬 데이터
	//------------------------------------------------------
	map<int16, st_AttackSkillData*> _ArcherAttackSkillDatas;
	map<int16, st_TacTicSkillData*> _ArcherTacTicSkillDatas;
	map<int16, st_BufSkillData*> _ArcherBufSkillDatas;

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

	st_SkillData* FindSkillData(en_SkillMediumCategory FindSkillMediumCategory, en_SkillType FindSkillType);
	st_ObjectStatusData* FindObjectStatusData(en_GameObjectType GameObjectType, int16 Level);
	st_ItemData* FindItemData(en_SmallItemCategory FindItemCategory);
};