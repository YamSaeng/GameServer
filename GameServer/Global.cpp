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
		G_NetworkManager = new CNetworkManager();

		// ������ ������ �Ľ�
		G_Datamanager->LoadDataItem(L"ItemData.json");
		
		// ��ų ������ �Ľ�
		G_Datamanager->LoadDataPublicSkill(L"PublicSkillDatas.json");		
		// ���� ��ų ������ �Ľ�
		G_Datamanager->LoadDataFightSkill(L"FightSkillDatas.json");	
		// ��� ��ų ������ �Ľ�
		G_Datamanager->LoadDataProtectionSkill(L"ProtectionSkillDatas.json");
		// �ϻ� ��ų ������ �Ľ�
		G_Datamanager->LoadDataAssassinationSkill(L"AssassinationSkillDatas.json");
		// ���� ��ų ������ �Ľ�
		G_Datamanager->LoadDataSpellSkill(L"SpellSkillDatas.json");
		// ��� ��ų ������ �Ľ�
		G_Datamanager->LoadDataShootingSkill(L"ShootingSkillDatas.json");
		// ���� ��ų ������ �Ľ�
		G_Datamanager->LoadDataDisCiplineSkill(L"DisciplineSkillDatas.json");
		// ���� ��ų ������ �Ľ�
		G_Datamanager->LoadDataMonsterSkill(L"MonsterSkillDatas.json");
		
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
		// ��� ������ �Ľ�
		G_Datamanager->LoadDataDropItem(L"DropItemDatas.json");
		// ������ ������ �Ľ�
		G_Datamanager->LoadDataCrafting(L"CraftingData.json");
		// ���۴� ���۹� ������ �Ľ�
		G_Datamanager->LoadDataCraftingTable(L"CraftingTableData.json");
		// ���� ������ �Ľ�
		G_Datamanager->LoadDataMerchant(L"MerchantItemData.json");
		// ĳ���� ������ ������ �Ľ�
		G_Datamanager->LoadDataLevel(L"CharacterLevelingData.json");
		// �� ���� ������ �Ľ�
		G_Datamanager->LoadDataMapInfo(L"MapInfoData.json");
		// �޴� ������ �Ľ�
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