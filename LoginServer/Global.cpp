#include "pch.h"
#include "Global.h"
#include "DBConnectionPool.h"

CDBConnectionPool* G_DBConnectionPool = nullptr;
CLog* G_Logger = nullptr;

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
	}

	~CGlobal()
	{
		delete G_DBConnectionPool;
		delete G_Logger;		
	}
} G_Global;