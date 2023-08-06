#include "pch.h"
#include "Global.h"
#include "DBConnectionPool.h"
#include "DataManager.h"
#include "MapManager.h"
#include "ObjectManager.h"
#include "NetworkManager.h"

CDBConnectionPool* G_DBConnectionPool = nullptr;
CLog* G_Logger = nullptr;
CDataManager* G_Datamanager = nullptr;
CMapManager* G_MapManager = nullptr;
CObjectManager* G_ObjectManager = nullptr;
CNetworkManager* G_NetworkManager = nullptr;

//------------------------------------------------
// 각종 전역 클래스 (Manager) 관리
// 전역 클래스의 생성 및 삭제를 순서대로 관리한다.
//------------------------------------------------
class CGlobal
{
public:
	CGlobal()
	{
		setlocale(LC_ALL, "Korean");
		G_DBConnectionPool = new CDBConnectionPool();
		G_DBConnectionPool->Init(1000);
		G_Logger = new CLog();
		G_ObjectManager = new CObjectManager();
		G_Datamanager = new CDataManager();
		G_NetworkManager = new CNetworkManager();

		// 아이템 데이터 파싱
		G_Datamanager->LoadDataItem(L"ItemData.json");
		
		// 스킬 데이터 파싱
		G_Datamanager->LoadDataPublicSkill(L"PublicSkillDatas.json");		
		// 격투 스킬 데이터 파싱
		G_Datamanager->LoadDataFightSkill(L"FightSkillDatas.json");	
		// 방어 스킬 데이터 파싱
		G_Datamanager->LoadDataProtectionSkill(L"ProtectionSkillDatas.json");
		// 암살 스킬 데이터 파싱
		G_Datamanager->LoadDataAssassinationSkill(L"AssassinationSkillDatas.json");
		// 마법 스킬 데이터 파싱
		G_Datamanager->LoadDataSpellSkill(L"SpellSkillDatas.json");
		// 사격 스킬 데이터 파싱
		G_Datamanager->LoadDataShootingSkill(L"ShootingSkillDatas.json");
		// 수양 스킬 데이터 파싱
		G_Datamanager->LoadDataDisCiplineSkill(L"DisciplineSkillDatas.json");
		// 몬스터 스킬 데이터 파싱
		G_Datamanager->LoadDataMonsterSkill(L"MonsterSkillDatas.json");
		
		// 플레이어 캐릭터 스테이터스 데이터 파싱
		G_Datamanager->LoadDataPlayerCharacterStatus(L"PlayerCharacterStatus.json");
		// 몬스터 데이터 파싱
		G_Datamanager->LoadDataMonster(L"MonsterData.json");
		// 몬스터 어그로 데이터 파싱
		G_Datamanager->LoadDataMonsterAggro(L"MonsterAggroData.json");
		// 환경 오브젝트 데이터 파싱
		G_Datamanager->LoadDataEnvironment(L"EnvironmentData.json");
		// 작물 데이터 파싱
		G_Datamanager->LoadDataCrop(L"CropData.json");
		// 드랍 데이터 파싱
		G_Datamanager->LoadDataDropItem(L"DropItemDatas.json");
		// 제작템 데이터 파싱
		G_Datamanager->LoadDataCrafting(L"CraftingData.json");
		// 제작대 제작법 데이터 파싱
		G_Datamanager->LoadDataCraftingTable(L"CraftingTableData.json");
		// 상인 데이터 파싱
		G_Datamanager->LoadDataMerchant(L"MerchantItemData.json");
		// 캐릭터 레벨링 데이터 파싱
		G_Datamanager->LoadDataLevel(L"CharacterLevelingData.json");
		// 맵 정보 데이터 파싱
		G_Datamanager->LoadDataMapInfo(L"MapInfoData.json");
		// 메뉴 데이터 파싱
		G_Datamanager->LoadDataOptionInfo(L"OptionData.json");

		G_MapManager = new CMapManager();			
	}

	~CGlobal()
	{
		delete G_DBConnectionPool;
		delete G_Logger;
		delete G_Datamanager;
		delete G_MapManager;
		delete G_ObjectManager;
	}
} G_Global;