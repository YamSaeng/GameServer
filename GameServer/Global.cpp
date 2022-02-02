#include "pch.h"
#include "Global.h"
#include "DBConnectionPool.h"
#include "DataManager.h"
#include "ChannelManager.h"
#include "ObjectManager.h"

CDBConnectionPool* G_DBConnectionPool = nullptr;
CLog* G_Logger = nullptr;
CDataManager* G_Datamanager = nullptr;
CChannelManager* G_ChannelManager = nullptr;
CObjectManager* G_ObjectManager = nullptr;

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
		G_Datamanager = new CDataManager();
		// 아이템 데이터 파싱
		G_Datamanager->LoadDataItem(L"ItemData.json");
		// 스킬 데이터 파싱
		G_Datamanager->LoadDataPublicSkill(L"PublicSkillDatas.json");
		G_Datamanager->LoadDataWarriorSkill(L"WarriorSkillDatas.json");
		G_Datamanager->LoadDataShamanSkill(L"ShamanSkillDatas.json");
		G_Datamanager->LoadDataTaioistSkill(L"TaioistSkillDatas.json");
		G_Datamanager->LoadDataThiefSkill(L"ThiefSkillDatas.json");
		G_Datamanager->LoadDataArcherSkill(L"ArcherSkillDatas.json");
		// 플레이어 캐릭터 스테이터스 데이터 파싱
		G_Datamanager->LoadDataPlayerCharacterStatus(L"PlayerCharacterStatus.json");
		// 몬스터 데이터 파싱
		G_Datamanager->LoadDataMonster(L"MonsterData.json");
		// 환경 오브젝트 데이터 파싱
		G_Datamanager->LoadDataEnvironment(L"EnvironmentData.json");
		// 제작템 데이터 파싱
		G_Datamanager->LoadDataCrafting(L"CraftingData.json");
		G_Datamanager->LoadDataLevel(L"CharacterLevelingData.json");

		G_ChannelManager = new CChannelManager();
		G_ChannelManager->Add(1);

		G_ObjectManager = new CObjectManager();
	}

	~CGlobal()
	{
		delete G_DBConnectionPool;
		delete G_Logger;
		delete G_Datamanager;
		delete G_ChannelManager;
		delete G_ObjectManager;
	}
} G_Global;