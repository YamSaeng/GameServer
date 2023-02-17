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
	map<int16, st_ItemInfo*> _GeneralMerchantItems;

	map<int32, st_StatInfo*> _PlayerStatus;

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
	map<int16, st_SkillInfo*> _PublicSkillDatas;	
	//------------------------------------------------------
	// 격투 스킬 데이터
	//------------------------------------------------------
	map<int16, st_SkillInfo*> _FightSkillDatas;	
	//------------------------------------------------------
	// 방어 스킬 데이터
	//------------------------------------------------------
	map<int16, st_SkillInfo*> _ProtectionSkillDatas;	
	//------------------------------------------------------
	// 암살 스킬 데이터
	//------------------------------------------------------
	map<int16, st_SkillInfo*> _AssassinationSkillDatas;	
	//------------------------------------------------------
	// 마법 스킬 데이터
	//------------------------------------------------------
	map<int16, st_SkillInfo*> _SpellSkillDatas;	
	//------------------------------------------------------
	// 사격 스킬 데이터
	//------------------------------------------------------
	map<int16, st_SkillInfo*> _ShootingSkillDatas;	
	//------------------------------------------------------
	// 수양 스킬 데이터
	//------------------------------------------------------
	map<int16, st_SkillInfo*> _DisciplineSkillDatas;	

	// 슬라임 스킬 데이터
	map<int16, st_SkillInfo*> _SlimeSkillDatas;	

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
	void LoadDataFightSkill(wstring LoadFileName);
	void LoadDataProtectionSkill(wstring LoadFileName);
	void LoadDataAssassinationSkill(wstring LoadFileName);	
	void LoadDataSpellSkill(wstring LoadFileName);
	void LoadDataShootingSkill(wstring LoadFileName);	
	void LoadDataDisCiplineSkill(wstring LoadFileName);	

	void LoadDataMonsterSkill(wstring LoadFileName);

	void LoadDataEnvironment(wstring LoadFileName);
	void LoadDataCrop(wstring LoadFileName);
	void LoadDataCrafting(wstring LoadFileName);
	void LoadDataCraftingTable(wstring LoadFileName);
	void LoadDataMerchant(wstring LoadFileName);
	void LoadDataMapInfo(wstring LoadFileName);
	void LoadDataOptionInfo(wstring LoadFileName);

	st_SkillInfo* FindSkillData(en_SkillType FindSkillType);
	st_StatInfo* FindObjectStatusData(en_GameObjectType GameObjectType, int16 Level);
	st_ItemInfo* FindItemData(en_SmallItemCategory FindItemCategory);	
	int32 FindMonsterExperienceData(en_GameObjectType MonsterGameObjectType);
};