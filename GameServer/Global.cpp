#include "pch.h"
#include "Global.h"
#include "DBConnectionPool.h"
#include "DataManager.h"


CDBConnectionPool* G_DBConnectionPool = nullptr;
CLog* G_Logger = nullptr;
CDataManager* G_Datamanager = nullptr;
//------------------------------------------------
// 각종 전역 클래스 (Manager) 관리
// 전역 클래스의 생성 및 삭제를 순서대로 관리한다.
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