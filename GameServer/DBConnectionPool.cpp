#include "pch.h"
#include "DBConnectionPool.h"
#include "XmlParser.h"

CDBConnectionPool::CDBConnectionPool()
{
	InitializeCriticalSection(&_CS);
}

CDBConnectionPool::~CDBConnectionPool()
{
	DeleteCriticalSection(&_CS);
	Clear();
}

bool CDBConnectionPool::Init(int32 ConnectionCount)
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

	CXmlNode Root;
	CXmlParser Parser;

	if (Parser.ParseFromFile(L"DBConfig.xml", Root) == false)
	{
		wprintf(L"DBCongfig.xml�� �� �� �����ϴ�.");
		return false;
	}

	vector<CXmlNode> DBs = Root.FindChildren(L"DB");
	for (int32 i = 0; i < DBs.size(); i++)
	{
		wstring DBName = DBs[i].GetStringAttr(L"name");

		CXmlNode Option = DBs[i].FindChild(L"Option");
		wstring ConnectionString = Option.GetStringAttr(L"ConnectionString");

		if (DBName == L"AccountDB")
		{
			// �Է¹��� ConnectionCount��ŭ DB Connection����
			for (int32 i = 0; i < ConnectionCount; i++)
			{
				// �����ؼ�
				CDBConnection* Connection = new CDBConnection();

				// DB�� ����
				if (Connection->Connect(_SqlEnvironment, ConnectionString.c_str()) == false)
				{
					return false;
				}

				// AccountDB �迭�� ����
				_AccountDBConnections.push_back(Connection);
			}
		}
		else if (DBName == L"GameDB")
		{
			for (int32 i = 0; i < ConnectionCount; i++)
			{
				CDBConnection* Connection = new CDBConnection();

				if (Connection->Connect(_SqlEnvironment, ConnectionString.c_str()) == false)
				{
					return false;
				}

				// GameDB �迭�� ����
				_GameDBConnections.push_back(Connection);
			}
		}
		else if (DBName == L"SharedDB")
		{
			for (int32 i = 0; i < ConnectionCount; i++)
			{
				CDBConnection* Connection = new CDBConnection();

				if (Connection->Connect(_SqlEnvironment, ConnectionString.c_str()) == false)
				{
					return false;
				}

				// GameDB �迭�� ����
				_TokenDBConnections.push_back(Connection);
			}
		}
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

	// AccountDBConnections ����
	for (CDBConnection* AccountDBConnection : _AccountDBConnections)
	{
		delete AccountDBConnection;
	}

	// GameDBConnections ����
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

	EnterCriticalSection(&_CS);

	switch (ConnectDBName)
	{
	case en_DBConnect::ACCOUNT:
		if (_AccountDBConnections.empty() == true)
		{
			break;
		}

		DBConnection = _AccountDBConnections.back();
		_AccountDBConnections.pop_back();
		break;
	case en_DBConnect::GAME:
		if (_GameDBConnections.empty() == true)
		{
			break;
		}

		DBConnection = _GameDBConnections.back();
		_GameDBConnections.pop_back();
		break;
	case en_DBConnect::TOKEN:
		if (_TokenDBConnections.empty() == true)
		{
			break;
		}

		DBConnection = _TokenDBConnections.back();
		_TokenDBConnections.pop_back();
		break;
	}

	LeaveCriticalSection(&_CS);

	return DBConnection;
}

void CDBConnectionPool::Push(en_DBConnect InputConnectDBName, CDBConnection* DBConnection)
{
	EnterCriticalSection(&_CS);

	switch (InputConnectDBName)
	{
	case en_DBConnect::ACCOUNT:
		_AccountDBConnections.push_back(DBConnection);
		break;
	case en_DBConnect::GAME:
		_GameDBConnections.push_back(DBConnection);
		break;
	case en_DBConnect::TOKEN:
		_TokenDBConnections.push_back(DBConnection);
		break;
	}

	LeaveCriticalSection(&_CS);
}