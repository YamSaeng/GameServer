#include "pch.h"
#include "Global.h"
#include "DBConnectionPool.h"
#include "DataManager.h"
#include "ChannelManager.h"


CDBConnectionPool* G_DBConnectionPool = nullptr;
CLog* G_Logger = nullptr;
CDataManager* G_Datamanager = nullptr;
CChannelManager* G_ChannelManager = nullptr;

//------------------------------------------------
// ���� ���� Ŭ���� (Manager) ����
// ���� Ŭ������ ���� �� ������ ������� �����Ѵ�.
//------------------------------------------------
class CGlobal
{
public:
	CGlobal()
	{
		G_DBConnectionPool = new CDBConnectionPool();
		G_DBConnectionPool->Init(50);
		G_Logger = new CLog();
		G_Datamanager = new CDataManager();
		G_Datamanager->LoadDataItem(L"ItemData.json");
		G_Datamanager->LoadDataStatus(L"StatusData.json");
		G_Datamanager->LoadDataMonster(L"MonsterData.json");
		
		G_ChannelManager = new CChannelManager();	
		G_ChannelManager->Add(1);
	}

	~CGlobal()
	{
		delete G_DBConnectionPool;
		delete G_Logger;
		delete G_Datamanager;
	}
} G_Global;