#include "pch.h"
#include "DBConnectionPool.h"
#include "XmlParser.h"

CDBConnectionPool::CDBConnectionPool()
{

}

CDBConnectionPool::~CDBConnectionPool()
{
	Clear();
}

bool CDBConnectionPool::Init(int32 ConnectionCount)
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

	CXmlNode Root;
	CXmlParser Parser;

	if (Parser.ParseFromFile(L"DBConfig.xml", Root) == false)
	{
		wprintf(L"DBCongfig.xml를 열 수 없습니다.");
		return false;
	}

	vector<CXmlNode> DBs = Root.FindChildren(L"DB");
	for (int32 i = 0; i < DBs.size(); i++)
	{
		wstring DBName = DBs[i].GetStringAttr(L"name");
		if (DBName == L"AccountDB")
		{ 
			CXmlNode Option = DBs[i].FindChild(L"Option");
			wstring ConnectionString = Option.GetStringAttr(L"ConnectionString");

			// 입력받은 ConnectionCount만큼 DB Connection생성
			for (int32 i = 0; i < ConnectionCount; i++)
			{
				// 생성해서
				CDBConnection* Connection = new CDBConnection();

				// DB에 연결
				if (Connection->Connect(_SqlEnvironment, ConnectionString.c_str()) == false)
				{
					return false;
				}

				// AccountDB 배열에 저장
				_AccountDBConnections.push_back(Connection);
			}
		}
		else if (DBName == L"GameDB")
		{
			CXmlNode Option = DBs[i].FindChild(L"Option");
			wstring ConnectionString = Option.GetStringAttr(L"ConnectionString");

			for (int32 i = 0; i < ConnectionCount; i++)
			{
				CDBConnection* Connection = new CDBConnection();

				if (Connection->Connect(_SqlEnvironment, ConnectionString.c_str()) == false)
				{
					return false;
				}

				// GameDB 배열에 저장
				_GameDBConnections.push_back(Connection);
			}
		}
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

	// AccountDBConnections 정리
	for (CDBConnection* AccountDBConnection : _AccountDBConnections)
	{
		delete AccountDBConnection;
	}

	// GameDBConnections 정리
	for (CDBConnection* GameDBConnection : _GameDBConnections)
	{
		delete GameDBConnection;
	}

	_AccountDBConnections.clear();
	_GameDBConnections.clear();
}

CDBConnection* CDBConnectionPool::Pop(en_DBConnect ConnectDBName)
{
	CDBConnection* DBConnection = nullptr;

	switch (ConnectDBName)
	{
	case en_DBConnect::ACCOUNT:
		if (_AccountDBConnections.empty() == true)
		{
			break;
		}

		DBConnection = _AccountDBConnections.back();
		_AccountDBConnections.pop_back();
		return DBConnection;
	case en_DBConnect::GAME:
		if (_GameDBConnections.empty() == true)
		{
			break;
		}

		DBConnection = _GameDBConnections.back();
		_GameDBConnections.pop_back();
		return DBConnection;
	}	
		
	return DBConnection;
}

void CDBConnectionPool::Push(en_DBConnect InputConnectDBName, CDBConnection* DBConnection)
{
	switch (InputConnectDBName)
	{
	case en_DBConnect::ACCOUNT:
		_AccountDBConnections.push_back(DBConnection);
		break;
	case en_DBConnect::GAME:
		_GameDBConnections.push_back(DBConnection);
		break;
	}
}
