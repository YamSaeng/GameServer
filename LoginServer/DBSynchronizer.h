#pragma once
#include "DBConnection.h"
#include "DBModel.h"

using namespace DBModel;

// PrimaryKey�� �̸��� �����Ҷ� ������ ����
// DB ������Ʈ ������ Dropindex -> AddColumn -> DropColumn ������ ���ְ�
// ���� Primary Key�� ������ ���� ��Ȳ���� AddColumn���� Primary Key �� �߰��Ϸ��� �ϴϱ� ����
//-------------------------------------------------
// DBSynchronizer
//  xml�� ������� DB������
//  DB�� �ִ� ������ �� �м��ؼ� ������Ʈ ���ش�.
//-------------------------------------------------

class DBSynchronizer
{
	enum
	{
		PROCEDURE_MAX_LEN = 10000
	};

	// DB ������Ʈ ����
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

	// ������Ʈ�� ���õ� ������ �����ϴ� �迭
	// CompareDBModel �Լ� �ȿ��� DB�� XMLDB�� ���Ͽ� �ٸ� �κ��� �߰��ϰ� ���������� �ٲ㼭 �迭�� ��´�.
	// �ٷ� ������ �������� �ʰ�, ��Ƽ� �����°��� ������ ���߱� ���ؼ��ε�
	// ���� Dependency�� �ɷ� ������, ���� �ϳ��� �����ϸ� �ٸ� �ְ� ������ ������ �ֱ⶧��
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