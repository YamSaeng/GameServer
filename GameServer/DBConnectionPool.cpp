#include "pch.h"
#include "DBConnectionPool.h"

CDBConnectionPool::CDBConnectionPool()
{

}

CDBConnectionPool::~CDBConnectionPool()
{
	Clear();
}

bool CDBConnectionPool::Connect(int32 ConnectionCount, const WCHAR* ConnectionString)
{
	// SQL_ENV ����
	if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_SqlEnvironment) != SQL_SUCCESS)
	{
		return false;
	}

	// ������ ���� SQLHandle�� �̿��� ODBC�� ������ ����
	if (SQLSetEnvAttr(_SqlEnvironment, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0) != SQL_SUCCESS)
	{
		return false;
	}

	// �Է¹��� ConnectionCount��ŭ DB Connection����
	for (int32 i = 0; i < ConnectionCount; i++)
	{
		// �����ؼ�
		CDBConnection* Connection = new CDBConnection();
		// DB�� ����
		if (Connection->Connect(_SqlEnvironment, ConnectionString) == false)
		{
			return false;
		}

		// �迭�� ����
		_DBConnections.push_back(Connection);
	}

	return true;
}

void CDBConnectionPool::Clear()
{
	// SQL ȯ�溯�� ����
	if (_SqlEnvironment != SQL_NULL_HANDLE)
	{
		SQLFreeHandle(SQL_HANDLE_ENV, _SqlEnvironment);
		_SqlEnvironment = SQL_NULL_HANDLE;
	}

	// DBConnections ����
	for (CDBConnection* DBConnection : _DBConnections)
	{
		delete DBConnection;
	}

	_DBConnections.clear();
}

CDBConnection* CDBConnectionPool::Pop()
{
	// ����� ������ nullptr��ȯ
	if (_DBConnections.empty() == true)
	{
		return nullptr;
	}

	// �ϳ� ������ ��ȯ
	CDBConnection* DBConnection = _DBConnections.back();
	_DBConnections.pop_back();
	return DBConnection;
}

void CDBConnectionPool::Push(CDBConnection* DBConnection)
{
	// �迭�� ����
	_DBConnections.push_back(DBConnection);
}
