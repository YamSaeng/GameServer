#pragma once
#include "DBConnection.h"

//-------------------------------------------------
// DBConnectionPool
// 
// DBConnection을 배열에 보관후 필요할때마다
// 하나씩 꺼내서 사용한다.
//-------------------------------------------------
enum class en_DBConnect :int32
{
	ACCOUNT,
	GAME
};

class CDBConnectionPool
{
private:	
	// SQL 환경변수
	SQLHENV _SqlEnvironment = SQL_NULL_HANDLE;
	
	// DB 연결단위인 DBConnection을 저장해둘 배열
	vector<CDBConnection*> _AccountDBConnections; // AccountDB
	vector<CDBConnection*> _GameDBConnections;	  // GameDB

	CRITICAL_SECTION _CS;
public:
	CDBConnectionPool();
	~CDBConnectionPool();

	// ConnectionCount개 만큼 DB 연결시켜서 저장해둠
	bool Init(int32 ConnectionCount);

	// SQL 환경변수, vector _DBConnections 정리
	void Clear();

	// DBConnection배열에서 DBConnection하나를 꺼내옴
	CDBConnection* Pop(en_DBConnect ConnectDBName);
	// 사용했던 DBConnection을 배열에 다시 저장
	void Push(en_DBConnect InputConnectDBName, CDBConnection* DBConnection);
};

