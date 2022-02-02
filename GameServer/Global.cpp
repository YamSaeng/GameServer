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
		// ȯ�� ������Ʈ ������ �Ľ�
		G_Datamanager->LoadDataEnvironment(L"EnvironmentData.json");
		// ������ ������ �Ľ�
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