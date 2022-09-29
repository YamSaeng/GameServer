#pragma once
#include "DBConnection.h"
#include "DBModel.h"

using namespace DBModel;

//-------------------------------------------------
// DBSynchronizer
//  xml로 기록중인 DB정보와
//  DB에 있는 내용을 비교 분석해서 업데이트 해준다.
//-------------------------------------------------

class DBSynchronizer
{
	enum
	{
		PROCEDURE_MAX_LEN = 30000
	};

	// DB 업데이트 순서
	enum UpdateStep : uint8
	{
		DropIndex,
		AlterColumn,
		AddColumn,
		CreateTable,
		DefaultConstraint,
		CreateIndex,
		DropColumn,
		DropTable,
		StoredProcecure,

		Max
	};

	enum ColumnFlag : uint8
	{
		Type = 1 << 0,
		Nullable = 1 << 1,
		Identity = 1 << 2,
		Default = 1 << 3,
		Length = 1 << 4,
		CheckDependency = 1 << 5,
	};
private:
	CDBConnection& _dbConn;

	vector<CTable*> _xmlTables;
	vector<CProcedure*> _xmlProcedures;
	set<wstring> _xmlRemovedTables;

	vector<CTable*>	_dbTables;
	vector<CProcedure*> _dbProcedures;

	set<wstring> _dependentIndexes;

	// 업데이트에 관련된 쿼리를 보관하는 배열
	// CompareDBModel 함수 안에서 DB와 XMLDB를 비교하여 다른 부분을 발견하고 쿼리문으로 바꿔서 배열에 담는다.
	// 바로 쿼리를 날려주지 않고, 모아서 날리는것은 순서를 맞추기 위해서인데
	// 서로 Dependency가 걸려 있으면, 먼저 하나를 삭제하면 다른 애가 오류가 날수도 있기때문
	vector<wstring> _updateQueries[UpdateStep::Max];
private:
	void		ParseXmlDB(const WCHAR* Path);
	bool		GatherDBTables();
	bool		GatherDBIndexes();
	bool		GatherDBStoredProcedures();

	void		CompareDBModel();
	void		CompareTables(CTable* DBTable, CTable* XmlTable);
	void		CompareColumns(CTable* dbTable, CColumn* dbColumn, CColumn* xmlColumn);
	void		CompareStoredProcedures();

	void		ExecuteUpdateQueries();
public:
	DBSynchronizer(CDBConnection& Connection) : _dbConn(Connection) {}
	~DBSynchronizer();

	bool Synchronize(const WCHAR* Path);
};