#pragma once
#include "DBConnection.h"

//-------------------------------------------------
// DBConnectionPool
// 
// DBConnection�� �迭�� ������ �ʿ��Ҷ�����
// �ϳ��� ������ ����Ѵ�.
//-------------------------------------------------
enum en_DBConnect
{
	ACCOUNT,
	GAME
};

class CDBConnectionPool
{
private:
	// SQL ȯ�溯��
	SQLHENV _SqlEnvironment = SQL_NULL_HANDLE;
	
	// DB ��������� DBConnection�� �����ص� �迭
	vector<CDBConnection*> _AccountDBConnections; // AccountDB
	vector<CDBConnection*> _GameDBConnections;	  // GameDB
public:
	CDBConnectionPool();
	~CDBConnectionPool();

	// ConnectionCount�� ��ŭ DB ������Ѽ� �����ص�
	bool Init(int32 ConnectionCount);

	// SQL ȯ�溯��, vector _DBConnections ����
	void Clear();

	// DBConnection�迭���� DBConnection�ϳ��� ������
	CDBConnection* Pop(en_DBConnect ConnectDBName);
	// ����ߴ� DBConnection�� �迭�� �ٽ� ����
	void Push(en_DBConnect InputConnectDBName, CDBConnection* DBConnection);
};

