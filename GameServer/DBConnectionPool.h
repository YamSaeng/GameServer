#pragma once
#include "DBConnection.h"

//-------------------------------------------------
// DBConnectionPool
// 
// DBConnection�� �迭�� ������ �ʿ��Ҷ�����
// �ϳ��� ������ ����Ѵ�.
//-------------------------------------------------
class CDBConnectionPool
{
private:
	// SQL ȯ�溯��
	SQLHENV _SqlEnvironment = SQL_NULL_HANDLE;
	// DB ��������� DBConnection�� �����ص� �迭
	vector<CDBConnection*> _DBConnections;
public:
	CDBConnectionPool();
	~CDBConnectionPool();

	// ConnectionCount�� ��ŭ DB ������Ѽ� �����ص�	
	bool Connect(int32 ConnectionCount, const WCHAR* ConnectionString);

	// SQL ȯ�溯��, vector _DBConnections ����
	void Clear();

	// DBConnection�迭���� DBConnection�ϳ��� ������
	CDBConnection* Pop();
	// ����ߴ� DBConnection�� �迭�� �ٽ� ����
	void Push(CDBConnection* DBConnection);
};

