#include "pch.h"
#include "Global.h"
#include "DBConnectionPool.h"
#include "DataManager.h"
#include "MapManager.h"
#include "ObjectManager.h"

CDBConnectionPool* G_DBConnectionPool = nullptr;
CLog* G_Logger = nullptr;
CDataManager* G_Datamanager = nullptr;
CMapManager* G_MapManager = nullptr;
CObjectManager* G_ObjectManager = nullptr;

//------------------------------------------------
// ���� ���� Ŭ���� (Manager) ����
// ���� Ŭ������ ���� �� ������ ������� �����Ѵ�.
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
		// ������ ������ �Ľ�
		G_Datamanager->LoadDataItem(L"ItemData.json");
		// ��ų ������ �Ľ�
		G_Datamanager->LoadDataPublicSkill(L"PublicSkillDatas.json");
		G_Datamanager->LoadDataWarriorSkill(L"WarriorSkillDatas.json");
		G_Datamanager->LoadDataShamanSkill(L"ShamanSkillDatas.json");
		G_Datamanager->LoadDataTaioistSkill(L"TaioistSkillDatas.json");
		G_Datamanager->LoadDataThiefSkill(L"ThiefSkillDatas.json");
		G_Datamanager->LoadDataArcherSkill(L"ArcherSkillDatas.json");
		// �÷��̾� ĳ���� �������ͽ� ������ �Ľ�
		G_Datamanager->LoadDataPlayerCharacterStatus(L"PlayerCharacterStatus.json");
		// ���� ������ �Ľ�
		G_Datamanager->LoadDataMonster(L"MonsterData.json");
		// ���� ��׷� ������ �Ľ�
		G_Datamanager->LoadDataMonsterAggro(L"MonsterAggroData.json");
		// ȯ�� ������Ʈ ������ �Ľ�
		G_Datamanager->LoadDataEnvironment(L"EnvironmentData.json");
		// �۹� ������ �Ľ�
		G_Datamanager->LoadDataCrop(L"CropData.json");
		// ������ ������ �Ľ�
		G_Datamanager->LoadDataCrafting(L"CraftingData.json");
		G_Datamanager->LoadDataCraftingTable(L"CraftingTableData.json");
		G_Datamanager->LoadDataLevel(L"CharacterLevelingData.json");
		G_Datamanager->LoadDataMapInfo(L"MapInfoData.json");

		G_MapManager = new CMapManager();
		G_MapManager->MapSave();		
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