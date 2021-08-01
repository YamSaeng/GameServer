#include "pch.h"
#include "Global.h"
#include "DBConnectionPool.h"
#include "DataManager.h"


CDBConnectionPool* G_DBConnectionPool = nullptr;
CLog* G_Logger = nullptr;
CDataManager* G_Datamanager = nullptr;
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
		G_Datamanager->LoadData(L"ItemData.json");
	}

	~CGlobal()
	{
		delete G_DBConnectionPool;
		delete G_Logger;
		delete G_Datamanager;
	}
} G_Global;