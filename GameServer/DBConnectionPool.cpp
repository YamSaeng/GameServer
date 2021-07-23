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
	// SQL_ENV 설정
	if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_SqlEnvironment) != SQL_SUCCESS)
	{
		return false;
	}

	// 위에서 얻은 SQLHandle을 이용해 ODBC의 버전을 설정
	if (SQLSetEnvAttr(_SqlEnvironment, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0) != SQL_SUCCESS)
	{
		return false;
	}

	// 입력받은 ConnectionCount만큼 DB Connection생성
	for (int32 i = 0; i < ConnectionCount; i++)
	{
		// 생성해서
		CDBConnection* Connection = new CDBConnection();
		// DB에 연결
		if (Connection->Connect(_SqlEnvironment, ConnectionString) == false)
		{
			return false;
		}

		// 배열에 저장
		_DBConnections.push_back(Connection);
	}

	return true;
}

void CDBConnectionPool::Clear()
{
	// SQL 환경변수 정리
	if (_SqlEnvironment != SQL_NULL_HANDLE)
	{
		SQLFreeHandle(SQL_HANDLE_ENV, _SqlEnvironment);
		_SqlEnvironment = SQL_NULL_HANDLE;
	}

	// DBConnections 정리
	for (CDBConnection* DBConnection : _DBConnections)
	{
		delete DBConnection;
	}

	_DBConnections.clear();
}

CDBConnection* CDBConnectionPool::Pop()
{
	// 비워져 있으면 nullptr반환
	if (_DBConnections.empty() == true)
	{
		return nullptr;
	}

	// 하나 꺼내서 반환
	CDBConnection* DBConnection = _DBConnections.back();
	_DBConnections.pop_back();
	return DBConnection;
}

void CDBConnectionPool::Push(CDBConnection* DBConnection)
{
	// 배열에 저장
	_DBConnections.push_back(DBConnection);
}
