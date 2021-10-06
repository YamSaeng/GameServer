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
		G_DBConnectionPool->Init(100);
		G_Logger = new CLog();
		G_Datamanager = new CDataManager();
		G_Datamanager->LoadDataItem(L"ItemData.json");
		G_Datamanager->LoadDataStatus(L"StatusData.json");
		G_Datamanager->LoadDataMonster(L"MonsterData.json");
		G_Datamanager->LoadDataSkill(L"SkillData.json");
		G_Datamanager->LoadDataEnvironment(L"EnvironmentData.json");
		
		G_ChannelManager = new CChannelManager();	
		G_ChannelManager->Add(1);

		G_ObjectManager = new CObjectManager();		
		delete G_ObjectManager;
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