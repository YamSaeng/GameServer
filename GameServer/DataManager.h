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
	map<int16, st_PassiveSkillInfo*> _PublicPassiveSkillDatas;
	map<int16, st_AttackSkillInfo*> _PublicAttackSkillDatas;		
	map<int16, st_BufSkillInfo*> _PublicBufSkillDatas;
	//------------------------------------------------------
	// 격투 스킬 데이터
	//------------------------------------------------------
	map<int16, st_PassiveSkillInfo*> _FightPassiveSkillDatas;
	map<int16, st_AttackSkillInfo*> _FightAttackSkillDatas;		
	map<int16, st_BufSkillInfo*> _FightBufSkillDatas;	
	//------------------------------------------------------
	// 방어 스킬 데이터
	//------------------------------------------------------
	map<int16, st_PassiveSkillInfo*> _ProtectionPassiveSkillDatas;
	map<int16, st_AttackSkillInfo*> _ProtectionAttackSkillDatas;
	map<int16, st_BufSkillInfo*> _ProtectionBufSkillDatas;
	//------------------------------------------------------
	// 암살 스킬 데이터
	//------------------------------------------------------
	map<int16, st_PassiveSkillInfo*> _AssassinationPassiveSkillDatas;
	map<int16, st_AttackSkillInfo*> _AssassinationAttackSkillDatas;	
	map<int16, st_BufSkillInfo*> _AssassinationBufSkillDatas;	
	//------------------------------------------------------
	// 마법 스킬 데이터
	//------------------------------------------------------
	map<int16, st_PassiveSkillInfo*> _SpellPassiveSkillDatas;
	map<int16, st_AttackSkillInfo*> _SpellAttackSkillDatas;	
	map<int16, st_BufSkillInfo*> _SpellBufSkillDatas;
	//------------------------------------------------------
	// 사격 스킬 데이터
	//------------------------------------------------------
	map<int16, st_PassiveSkillInfo*> _ShootingPassiveSkillDatas;
	map<int16, st_AttackSkillInfo*> _ShootingAttackSkillDatas;	
	map<int16, st_BufSkillInfo*> _ShootingBufSkillDatas;
	//------------------------------------------------------
	// 수양 스킬 데이터
	//------------------------------------------------------
	map<int16, st_PassiveSkillInfo*> _DisciplinePassiveSkillDatas;
	map<int16, st_AttackSkillInfo*> _DisciplineAttackSkillDatas;	
	map<int16, st_HealSkillInfo*> _DisciplineHealSkillDatas;
	map<int16, st_BufSkillInfo*> _DisciplineBufSkillDatas;

	// 슬라임 스킬 데이터
	map<int16, st_AttackSkillInfo*> _SlimeAttackSkillDatas;

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
	st_ObjectStatusData* FindObjectStatusData(en_GameObjectType GameObjectType, int16 Level);
	st_ItemInfo* FindItemData(en_SmallItemCategory FindItemCategory);	
	int32 FindMonsterExperienceData(en_GameObjectType MonsterGameObjectType);
};