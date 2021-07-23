#include "pch.h"
#include "Global.h"
#include "DBConnectionPool.h"

CDBConnectionPool* G_DBConnectionPool = nullptr;
CLog* G_Logger = nullptr;
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
		G_Logger = new CLog();
	}

	~CGlobal()
	{
		delete G_DBConnectionPool;
		delete G_Logger;
	}
} G_Global;