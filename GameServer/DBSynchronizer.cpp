#include "pch.h"
#include "DBSynchronizer.h"
#include "DBBind.h"
#include "XmlParser.h"
#include <regex>

//---------------------------------
// DB�� ����Ǿ� �ִ� ���� �ܾ����
//---------------------------------
namespace SP
{
	// DB�� �ִ� ���̺�� �÷� ����� ��� �ܾ����
	const WCHAR* QTablesAndColumns =
		L"	SELECT c.object_id, t.name AS tableName, c.name AS columnName, c.column_id, c.user_type_id, c.max_length,"
		"		c.is_nullable, c.is_identity, CAST(ic.seed_value AS BIGINT) AS seedValue, CAST(ic.increment_value AS BIGINT) AS incValue,"
		"		c.default_object_id, dc.definition as defaultDefinition, dc.name as defaultConstraintName"
		"	FROM sys.columns AS c"
		"	JOIN sys.tables AS t"
		"		ON c.object_id = t.object_id"
		"	LEFT JOIN sys.default_constraints AS dc"
		"		ON c.default_object_id = dc.object_id"
		"	LEFT JOIN sys.identity_columns AS ic"
		"		ON c.object_id = ic.object_id AND c.column_id = ic.column_id"
		"	WHERE t.type = 'U'"
		"	ORDER BY object_id ASC, column_id ASC;";

	// DB ���̺�� �÷��� ���ε� ���Ѽ� �޸𸮷� ��������
	class GetDBTables : public CDBBind<0, 13>
	{
	public:
		GetDBTables(CDBConnection& conn) : CDBBind(conn, QTablesAndColumns) {}

		void Out_ObjectId(OUT int32& value) { BindCol(0, value); }
		template<int32 N> void Out_TableName(OUT WCHAR(&value)[N]) { BindCol(1, value); }
		template<int32 N> void Out_ColumnName(OUT WCHAR(&value)[N]) { BindCol(2, value); }
		void Out_ColumnId(OUT int32& value) { BindCol(3, value); }
		void Out_UserType(OUT int32& value) { BindCol(4, value); }
		void Out_MaxLength(OUT int32& value) { BindCol(5, value); }
		void Out_IsNullable(OUT bool& value) { BindCol(6, value); }
		void Out_IsIdentity(OUT bool& value) { BindCol(7, value); }
		void Out_SeedValue(OUT int64& value) { BindCol(8, value); }
		void Out_IncrementValue(OUT int64& value) { BindCol(9, value); }
		void Out_DefaultObjectId(OUT int32& value) { BindCol(10, value); }
		template<int32 N> void Out_DefaultDefinition(OUT WCHAR(&value)[N]) { BindCol(11, value); }
		template<int32 N> void Out_DefaultConstraintName(OUT WCHAR(&value)[N]) { BindCol(12, value); }
	};

	// �ε��� ��� ��������
	const WCHAR* QIndexes =
		L"	SELECT i.object_id, i.name as indexName, i.index_id, i.type, i.is_primary_key,"
		"		i.is_unique_constraint, ic.column_id, COL_NAME(ic.object_id, ic.column_id) as columnName"
		"	FROM sys.indexes AS i"
		"	JOIN sys.index_columns AS ic"
		"		ON i.object_id = ic.object_id AND i.index_id = ic.index_id"
		"	WHERE i.type > 0 AND i.object_id IN(SELECT object_id FROM sys.tables WHERE type = 'U')"
		"	ORDER BY i.object_id ASC, i.index_id ASC;";

	// �ε��� ��� �޸𸮷� ��������
	class GetDBIndexes : public CDBBind<0, 8>
	{
	public:
		GetDBIndexes(CDBConnection& conn) : CDBBind(conn, QIndexes) {}

		void Out_ObjectId(OUT int32& value) { BindCol(0, value); }
		template<int32 N> void Out_IndexName(OUT WCHAR(&value)[N]) { BindCol(1, value); }
		void Out_IndexId(OUT int32& value) { BindCol(2, value); }
		void Out_IndexType(OUT int32& value) { BindCol(3, value); }
		void Out_IsPrimaryKey(OUT bool& value) { BindCol(4, value); }
		void Out_IsUniqueConstraint(OUT bool& value) { BindCol(5, value); }
		void Out_ColumnId(OUT int32& value) { BindCol(6, value); }
		template<int32 N> void Out_ColumnName(OUT WCHAR(&value)[N]) { BindCol(7, value); }
	};

	// ����Ǿ� �ִ� ���ν����� ��� �����´�.
	const WCHAR* QStoredProcedures =
		L"	SELECT name, OBJECT_DEFINITION(object_id) AS body FROM sys.procedures;";

	// ���ν��� ��� �޸𸮷� ��������
	class GetDBStoredProcedures : public CDBBind<0, 2>
	{
	public:
		GetDBStoredProcedures(CDBConnection& conn) : CDBBind(conn, QStoredProcedures) {}

		template<int32 N> void Out_Name(OUT WCHAR(&value)[N]) { BindCol(0, value); }
		void Out_Body(OUT WCHAR* value, int32 len) { BindCol(1, value, len); }
	};
}

DBSynchronizer::~DBSynchronizer()
{
}

bool DBSynchronizer::Synchronize(const WCHAR* path)
{
	// xml���Ͽ� �ִ� �����͵��� �о �޸𸮿� ����
	ParseXmlDB(path);

	// DB�� ��ϵǾ� �ִ� ���̺�, �ε���, ���ν��� ������ �ܾ�ͼ� �޸𸮿� ����
	GatherDBTables();
	GatherDBIndexes();
	GatherDBStoredProcedures();

	// xml������ DB������ DB���� �о���� ������ ���ؼ� 
	// ������ ���� or �߰��� ������ ������ DB�� ������Ʈ �Ѵ�. xml������ �ֽ� DB����	
	CompareDBModel(); // �̰����� ������Ʈ �� �׸��� �����ϰ�
	ExecuteUpdateQueries(); // ���⿡�� ������Ʈ�� �����Ѵ�.

	return true;
}

void DBSynchronizer::ParseXmlDB(const WCHAR* path)
{
	CXmlNode root;
	CXmlParser parser;
	ASSERT_CRASH(parser.ParseFromFile(path, OUT root));

	// <Table>���� ã�Ƽ� Tables�� ����
	vector<CXmlNode> tables = root.FindChildren(L"Table");
	for (CXmlNode& table : tables)
	{
		CTable* Table = new CTable();

		// <Table>�� name Attribute(�Ӽ�) �����ͼ� �����ص�
		Table->_name = table.GetStringAttr(L"name");

		// <Table>�� <Column>���� ã�Ƽ� Columns�� ����
		vector<CXmlNode> columns = table.FindChildren(L"Column");
		for (CXmlNode& ColumnNode : columns)
		{
			CColumn* Column = new CColumn();

			// <Column>�� name, typetext, type, nuillable �Ӽ��� �����ͼ� �����ص�
			Column->_name = ColumnNode.GetStringAttr(L"name");
			Column->_typeText = ColumnNode.GetStringAttr(L"type");
			Column->_type = DBModel::Helpers::WStringToDataType(Column->_typeText.c_str(), OUT Column->_maxLength);
			ASSERT_CRASH(Column->_type != DBModel::DataType::None);
			Column->_nullable = !ColumnNode.GetBoolAttr(L"notnull", false);

			// <Column>�� Identity �Ӽ��� �����´�.
			const WCHAR* identityStr = ColumnNode.GetStringAttr(L"identity");
			if (::wcslen(identityStr) > 0)
			{
				// ����ǥ���� ���� (����,����)
				std::wregex pt(L"(\\d+),(\\d+)");
				std::wcmatch match;

				// ����ǥ����(����,����)���� ���� �˻��ؼ� match�� ����
				ASSERT_CRASH(std::regex_match(identityStr, OUT match, pt));
				Column->_identity = true;
				Column->_seedValue = _wtoi(match[1].str().c_str());
				Column->_incrementValue = _wtoi(match[2].str().c_str());
			}

			// <Column>�� default�Ӽ��� �����ͼ� ����
			Column->_default = ColumnNode.GetStringAttr(L"default");
			// �ϼ��� �÷��� ����
			Table->_columns.push_back(Column);
		}

		// <Table>�� <Index>���� ã�Ƽ� Indexes�� ����
		vector<CXmlNode> indexes = table.FindChildren(L"Index");
		for (CXmlNode& IndexNode : indexes)
		{
			CIndex* i = new CIndex();

			// Ÿ�� �о��
			const WCHAR* typeStr = IndexNode.GetStringAttr(L"type");

			// Ÿ���� clustered���� nonclustered���� Ȯ��
			if (::_wcsicmp(typeStr, L"clustered") == 0)
			{
				i->_type = DBModel::IndexType::Clustered;
			}
			else if (::_wcsicmp(typeStr, L"nonclustered") == 0)
			{
				i->_type = DBModel::IndexType::NonClustered;
			}
			else
			{
				CRASH("Invalid Index Type");
			}

			// <PrimaryKey>�� �ִ��� Ȯ��
			i->_primaryKey = IndexNode.FindChild(L"PrimaryKey").IsValid();
			// <UniqueKey>�� �ִ��� Ȯ��
			i->_uniqueConstraint = IndexNode.FindChild(L"UniqueKey").IsValid();

			// <Table> - <Index> - <Column>���� ã�Ƽ� Columns�� ����
			vector<CXmlNode> columns = IndexNode.FindChildren(L"Column");
			// �о���� Column ��ȯ�ϸ鼭
			for (CXmlNode& column : columns)
			{
				// �÷� �̸��� ������ ��
				const WCHAR* nameStr = column.GetStringAttr(L"name");
				// �÷� ����
				CColumn* c = Table->FindColumn(nameStr);
				ASSERT_CRASH(c != nullptr);
				// Index�� columns�� ����
				i->_columns.push_back(c);
			}

			// �ϼ��� Index�� ����
			Table->_indexes.push_back(i);
		}

		// ���������� �ϼ��� T (���̺�) ����
		_xmlTables.push_back(Table);
	}

	// <Procedure>���� ã�Ƽ� �������
	vector<CXmlNode> procedures = root.FindChildren(L"Procedure");
	for (CXmlNode& procedure : procedures)
	{
		CProcedure* p = new CProcedure();
		p->_name = procedure.GetStringAttr(L"name");
		p->_body = procedure.FindChild(L"Body").GetStringValue();

		vector<CXmlNode> params = procedure.FindChildren(L"Param");
		for (CXmlNode& paramNode : params)
		{
			DBModel::st_Param param;
			param._name = paramNode.GetStringAttr(L"name");
			param._type = paramNode.GetStringAttr(L"type");
			p->_parameters.push_back(param);
		}

		_xmlProcedures.push_back(p);
	}

	// ������ ���̺� ���
	// CompareTable�� ���� DB�� xmlDB�� ���� ������ ���̺��� ������� 
	// ������ ���̺� ��Ͽ� ������ ���̺��� �־�� ���������� �ش� ���̺��� �����Ѵ�.
	vector<CXmlNode> removedTables = root.FindChildren(L"RemovedTable");
	for (CXmlNode& removedTable : removedTables)
	{
		_xmlRemovedTables.insert(removedTable.GetStringAttr(L"name"));
	}
}

// DB�� ��ϵǾ� �ִ� ���̺�, �ε���, ���ν��� ������ �ܾ�ͼ� �޸𸮿� ����
bool DBSynchronizer::GatherDBTables()
{
	int32 objectId;
	WCHAR tableName[101] = { 0 };
	WCHAR columnName[101] = { 0 };
	int32 columnId;
	int32 userTypeId;
	int32 maxLength;
	bool isNullable;
	bool isIdentity;
	int64 seedValue;
	int64 incValue;
	int32 defaultObjectId;
	WCHAR defaultDefinition[101] = { 0 };
	WCHAR defaultConstraintName[101] = { 0 };

	// ���̺� ��ϵǾ� �ִ� ������ DB�� ���� �����´�.
	SP::GetDBTables getDBTables(_dbConn);

	getDBTables.Out_ObjectId(OUT objectId);
	getDBTables.Out_TableName(OUT tableName);
	getDBTables.Out_ColumnName(OUT columnName);
	getDBTables.Out_ColumnId(OUT columnId);
	getDBTables.Out_UserType(OUT userTypeId);
	getDBTables.Out_MaxLength(OUT maxLength);
	getDBTables.Out_IsNullable(OUT isNullable);
	getDBTables.Out_IsIdentity(OUT isIdentity);
	getDBTables.Out_SeedValue(OUT seedValue);
	getDBTables.Out_IncrementValue(OUT incValue);
	getDBTables.Out_DefaultObjectId(OUT defaultObjectId);
	getDBTables.Out_DefaultDefinition(OUT defaultDefinition);
	getDBTables.Out_DefaultConstraintName(OUT defaultConstraintName);

	// DB�� ����
	if (getDBTables.Execute() == false)
	{
		return false;
	}

	// ������ ��������
	while (getDBTables.Fetch())
	{
		CTable* table;

		auto findTable = std::find_if(_dbTables.begin(), _dbTables.end(), [=](const CTable* table) { return table->_objectId == objectId; });
		if (findTable == _dbTables.end())
		{
			// �ߺ��Ǵ°� ������ ���� ����
			table = new CTable();
			table->_objectId = objectId;
			table->_name = tableName;
			_dbTables.push_back(table);
		}
		else
		{
			// �ߺ��Ǵ°� ������ �ִ��� ������ ������ ��
			table = *findTable;
		}

		// DB���� �о���� �÷� ���� ����
		CColumn* column = new CColumn();

		column->_name = columnName;
		column->_columnId = columnId;
		column->_type = static_cast<DBModel::DataType>(userTypeId);
		column->_typeText = DBModel::Helpers::DataTypeToWString(column->_type);
		column->_maxLength = (column->_type == DBModel::DataType::NVarChar ? maxLength / 2 : maxLength);
		column->_nullable = isNullable;
		column->_identity = isIdentity;
		column->_seedValue = (isIdentity ? seedValue : 0);
		column->_incrementValue = (isIdentity ? incValue : 0);

		// �÷��� ����Ʈ ���� 0���� Ŭ���
		if (defaultObjectId > 0)
		{
			column->_default = defaultDefinition;
			uint64 p = column->_default.find_first_not_of('(');
			column->_default = column->_default.substr(p, column->_default.size() - p * 2);
			column->_defaultConstraintName = defaultConstraintName;
		}

		table->_columns.push_back(column);
	}

	return true;
}

// DB�� �ִ� Index���� �ܾ����
bool DBSynchronizer::GatherDBIndexes()
{
	int32 objectId;
	WCHAR indexName[101] = { 0 };
	int32 indexId;
	int32 indexType;
	bool isPrimaryKey;
	bool isUniqueConstraint;
	int32 columnId;
	WCHAR columnName[101] = { 0 };

	// DB�� ��ϵǾ� �ִ� Index ������ �ܾ�´�.
	SP::GetDBIndexes getDBIndexes(_dbConn);

	getDBIndexes.Out_ObjectId(OUT objectId);
	getDBIndexes.Out_IndexName(OUT indexName);
	getDBIndexes.Out_IndexId(OUT indexId);
	getDBIndexes.Out_IndexType(OUT indexType);
	getDBIndexes.Out_IsPrimaryKey(OUT isPrimaryKey);
	getDBIndexes.Out_IsUniqueConstraint(OUT isUniqueConstraint);
	getDBIndexes.Out_ColumnId(OUT columnId);
	getDBIndexes.Out_ColumnName(OUT columnName);

	if (getDBIndexes.Execute() == false)
	{
		return false;
	}

	while (getDBIndexes.Fetch())
	{
		auto findTable = std::find_if(_dbTables.begin(), _dbTables.end(), [=](const CTable* table) { return table->_objectId == objectId; });

		ASSERT_CRASH(findTable != _dbTables.end());

		// ã�� ���̺��� �ε����� �ܾ��
		vector<CIndex*>& indexes = (*findTable)->_indexes;
		auto findIndex = std::find_if(indexes.begin(), indexes.end(), [indexId](const CIndex* index) { return index->_indexId == indexId; });
		if (findIndex == indexes.end())
		{
			// �ߺ��Ǵ°� ������ ���� ����
			CIndex* index = new CIndex();

			index->_name = indexName;
			index->_indexId = indexId;
			index->_type = static_cast<DBModel::IndexType>(indexType);
			index->_primaryKey = isPrimaryKey;
			index->_uniqueConstraint = isUniqueConstraint;

			indexes.push_back(index);
			findIndex = indexes.end() - 1;
		}

		// �ε����� �ɸ� column ã�Ƽ� �������ش�.
		vector<CColumn*>& columns = (*findTable)->_columns;
		auto findColumn = std::find_if(columns.begin(), columns.end(), [columnId](const CColumn* column) { return column->_columnId == columnId; });
		ASSERT_CRASH(findColumn != columns.end());
		(*findIndex)->_columns.push_back(*findColumn);
	}

	return true;
}

// DB�� �ִ� Procedure ���� �ܾ����
bool DBSynchronizer::GatherDBStoredProcedures()
{
	WCHAR name[101] = { 0 };
	vector<WCHAR> body(PROCEDURE_MAX_LEN);

	// DB���� ����Ǿ� �ִ� Procedure ���� �ܾ����
	SP::GetDBStoredProcedures getDBStoredProcedures(_dbConn);

	getDBStoredProcedures.Out_Name(OUT name);
	getDBStoredProcedures.Out_Body(OUT & body[0], PROCEDURE_MAX_LEN);

	if (getDBStoredProcedures.Execute() == false)
	{
		return false;
	}

	while (getDBStoredProcedures.Fetch())
	{
		CProcedure* proc = new CProcedure();
		proc->_name = name;
		proc->_fullBody = wstring(body.begin(), std::find(body.begin(), body.end(), 0));

		_dbProcedures.push_back(proc);
	}

	return true;
}

// �������� xml��� DB ����� ���� DB���� �о���� ������ ��
void DBSynchronizer::CompareDBModel()
{
	// ������Ʈ ��� �ʱ�ȭ.
	_dependentIndexes.clear();
	for (vector<wstring>& queries : _updateQueries)
	{
		queries.clear();
	}

	// XML�� �ִ� ����� �켱 ���� �´�.
	map<wstring, CTable*> xmlTableMap;
	for (CTable*& xmlTable : _xmlTables)
	{
		xmlTableMap[xmlTable->_name] = xmlTable;
	}

	// DB�� �����ϴ� ���̺���� ���鼭 XML�� ���ǵ� ���̺��� ���Ѵ�.
	// ���� ����, �߰� ������ ���̺��� �ִ��� Ȯ�� �ϴ� �κ�
	for (CTable*& dbTable : _dbTables)
	{
		// XmlTableMap���� DBTable�� name�� ������ Table�� ã�´�.
		// �ٽ� ����, DataBase�� �ִ� ���̺��� XML�� �ִ��� �Ǻ�
		auto findTable = xmlTableMap.find(dbTable->_name);
		// ã���� ���
		// ��, XML���� �ִ� ���
		if (findTable != xmlTableMap.end())
		{
			// ���̺��� ������ ��
			CTable* xmlTable = findTable->second;
			// ���̺��� ���Ѵ�. (���� ����, �߰�, ������ �÷��� �ִ��� Ȯ��)
			CompareTables(dbTable, xmlTable);
			// ó�� �Ϸ�� ���̺� ���� 
			xmlTableMap.erase(findTable);
		}
		else
		{
			// �� ã���� ���
			// ��, XML�� ���� ��� -> ���� �ؾ���
			// ���� ���� ���̺� ��Ͽ� �����Ϸ��� ���̺��� ������
			if (_xmlRemovedTables.find(dbTable->_name) != _xmlRemovedTables.end())
			{
				// �ܼ� ����ϰ�					
				G_Logger->WriteStdOut(en_Color::YELLOW, L"Removing Table : [dbo].[%s]\n", dbTable->_name.c_str());
				// DROP TABLE ������ ������ ����
				_updateQueries[UpdateStep::DropTable].push_back(DBModel::Helpers::Format(L"DROP TABLE [dbo].[%s]", dbTable->_name.c_str()));
			}
		}
	}

	// �ʿ��� ���ŵ��� ���� XML ���̺� ���Ǹ� ���� �߰�
	// -> DataBase���� ���� XMl�� �ִ� �׸���� �߰�
	for (auto& mapIt : xmlTableMap)
	{
		CTable*& xmlTable = mapIt.second;

		// �÷� �̸� ����
		wstring columnsStr;
		const int32 size = static_cast<int32>(xmlTable->_columns.size());
		for (int32 i = 0; i < size; i++)
		{
			if (i != 0)
			{
				columnsStr += L",";
			}

			columnsStr += L"\n\t";
			columnsStr += xmlTable->_columns[i]->CreateText();
		}

		// CREATE TABLE �ܼ� ����ؼ� �˷���
		G_Logger->WriteStdOut(en_Color::YELLOW, L"Creating Table : [dbo].[%s]\n", xmlTable->_name.c_str());
		// CREATE TABLE ������ ������ ����
		_updateQueries[UpdateStep::CreateTable].push_back(DBModel::Helpers::Format(L"CREATE TABLE [dbo].[%s] (%s)", xmlTable->_name.c_str(), columnsStr.c_str()));

		// XmlTable�� �ִ� �Ӽ� ���鼭
		for (CColumn*& xmlColumn : xmlTable->_columns)
		{
			// Defualt üũ 
			// ����� ������ �ѱ�
			if (xmlColumn->_default.empty())
			{
				continue;
			}

			// Default�� �Ⱥ���� ������ Default�� ���� ������ �ۼ��� ����
			_updateQueries[UpdateStep::DefaultConstraint].push_back(DBModel::Helpers::Format(L"ALTER TABLE [dbo].[%s] ADD CONSTRAINT [%s] DEFAULT (%s) FOR [%s]",
				xmlTable->_name.c_str(),
				DBModel::Helpers::Format(L"DF_%s_%s", xmlTable->_name.c_str(), xmlColumn->_name.c_str()).c_str(),
				xmlColumn->_default.c_str(),
				xmlColumn->_name.c_str()));
		}

		// XMLTable�� �ִ� �ε����� ���鼭
		for (CIndex*& xmlIndex : xmlTable->_indexes)
		{
			// INDEX ���� �ܼ� ����ؼ� �˷���
			G_Logger->WriteStdOut(en_Color::YELLOW, L"Creating Index : [%s] %s %s [%s]\n", xmlTable->_name.c_str(), xmlIndex->GetKeyText().c_str(), xmlIndex->GetTypeText().c_str(), xmlIndex->GetUniqueName().c_str());
			// �ε����� PrimaryKey or UniqueKey�� �ɷ�������
			if (xmlIndex->_primaryKey || xmlIndex->_uniqueConstraint)
			{
				// ������ �����Ǿ� �־��� �Ӽ��� PrimaryKey or Unique ���������� �ɾ �Ӽ��� ��������ִ� ������

				// PRIMARY KEY
				// ALTER TABLE [dbo].[Account] ADD CONSTRAINT [IX_Account_accountID] PRIMARY KEY CLUSTERED ([accountID])
				// UNIQUE
				// ALTER TABLE [dbo].[Account] ADD CONSTRAINT [IX_Account_testId_testId2] UNIQUE NONCLUSTERED ([testId],[testId2])
				_updateQueries[UpdateStep::CreateIndex].push_back(DBModel::Helpers::Format(
					L"ALTER TABLE [dbo].[%s] ADD CONSTRAINT [%s] %s %s (%s)",
					xmlTable->_name.c_str(), // ���̺� �̸�
					xmlIndex->CreateName(xmlTable->_name).c_str(), // �߰��� �ε���Ű�� �̸� ����
					xmlIndex->GetKeyText().c_str(),  // PRIMARYKEY ���� UNIQUEKEY ����
					xmlIndex->GetTypeText().c_str(), // �߰��� Ű�� CLUSTERED���� NONCLUSTERED ����
					xmlIndex->CreateColumnsText().c_str())); // PRIMARYKEY or UNIQUEKEY�� �߰��� �÷��� �̸� 
			}
			else
			{
				// �ε����� PrimaryKey or UniqueKey�� �ɷ����� ������

				// PrimaryKey or UniqueKey �� �����ؼ� �߰��ϴ� ������ �����ؼ� ����
				// CREATE CLUSTERED or NONCLUSTERED [IX_���̺��_�̸�] [dbo].[���̺��] (�÷��̸�)
				_updateQueries[UpdateStep::CreateIndex].push_back(DBModel::Helpers::Format(
					L"CREATE %s INDEX [%s] ON [dbo].[%s] (%s)",
					xmlIndex->GetTypeText().c_str(), // CLUSTERED ���� NONCLUSTERED ����
					xmlIndex->CreateName(xmlTable->_name).c_str(), // �߰��� �ε���Ű�� �̸� 
					xmlTable->_name.c_str(), // ���̺� �̸�
					xmlIndex->CreateColumnsText().c_str())); // �ε����� �߰��� �÷��� �̸�
			}
		}
	}

	// DB�� �ִ� Procedure�� xml Procedure ��
	CompareStoredProcedures();
}

void DBSynchronizer::ExecuteUpdateQueries()
{
	// ���� �Ǿ� �ִ� ���� ����
	for (int32 step = 0; step < UpdateStep::Max; step++)
	{
		for (wstring& query : _updateQueries[step])
		{
			_dbConn.UnBind();
			ASSERT_CRASH(_dbConn.Execute(query.c_str()));
		}
	}
}

// DB�� ����Ǿ� �ִ� ���̺�� XML�� ���ǵǾ� �ִ� ���̺��� ���Ѵ�.
void DBSynchronizer::CompareTables(CTable* dbTable, CTable* xmlTable)
{
	// XML�� �ִ� �÷� ����� ���� �´�.
	map<wstring, CColumn*> xmlColumnMap;
	for (CColumn*& xmlColumn : xmlTable->_columns)
	{
		xmlColumnMap[xmlColumn->_name] = xmlColumn;
	}

	// DB�� �����ϴ� ���̺� �÷����� ���鼭 XML�� ���ǵ� �÷���� ���Ѵ�.
	// ���� �߰���, ����, ������ �÷��� �ִ��� Ȯ���ϴ� �κ�
	for (CColumn*& DBColumn : dbTable->_columns)
	{
		// XmlColumnMap���� DBColumn�� name�� ������ Column�� ã�´�.
		// �ٽ� ����, DataBase�� �ִ� Column�� XML�� �ִ��� �Ǻ�
		auto findColumn = xmlColumnMap.find(DBColumn->_name);
		// ã���� ��� ��, XML���� �ִ� ���
		if (findColumn != xmlColumnMap.end())
		{
			// �÷��� ��������
			CColumn*& xmlColumn = findColumn->second;
			// �÷� ��
			CompareColumns(dbTable, DBColumn, xmlColumn);
			// ó�� �Ϸ�� �÷� ����
			xmlColumnMap.erase(findColumn);
		}
		else
		{
			// �� ã���� ��� ��, XML�� ���� ��� ( ���� �ؾ��� )
			G_Logger->WriteStdOut(en_Color::YELLOW, L"Dropping Column : [%s].[%s]\n", dbTable->_name.c_str(), DBColumn->_name.c_str());
			// ��������� �մ��� Ȯ��
			if (DBColumn->_defaultConstraintName.empty() == false)
			{
				//���� ���� ������ ���� ���� �����ϴ� ������ �ۼ��� ����
				_updateQueries[UpdateStep::DropColumn].push_back(DBModel::Helpers::Format(L"ALTER TABLE [dbo].[%s] DROP CONSTRAINT [%s]", dbTable->_name.c_str(), DBColumn->_defaultConstraintName.c_str()));
			}

			// ������ ���� �� ����
			_updateQueries[UpdateStep::DropColumn].push_back(DBModel::Helpers::Format(L"ALTER TABLE [dbo].[%s] DROP COLUMN [%s]", dbTable->_name.c_str(), DBColumn->_name.c_str()));
		}
	}

	// �ʿ��� ���ŵ��� ���� XML �÷��� ���� �߰�
	// -> DataBase���� ���� XMl�� �ִ� �׸���� �߰�
	for (auto& NewColumnIt : xmlColumnMap)
	{
		CColumn*& xmlColumn = NewColumnIt.second;
		DBModel::CColumn newColumn = *xmlColumn;
		newColumn._nullable = true;

		// �ַܼ� Adding Column �׸� �˷���
		G_Logger->WriteStdOut(en_Color::YELLOW, L"Adding Column : [%s].[%s]\n", dbTable->_name.c_str(), xmlColumn->_name.c_str());
		// �÷� �߰� ������ ���� �� ����
		_updateQueries[UpdateStep::AddColumn].push_back(DBModel::Helpers::Format(L"ALTER TABLE [dbo].[%s] ADD %s %s",
			dbTable->_name.c_str(), xmlColumn->_name.c_str(), xmlColumn->_typeText.c_str()));

		if (xmlColumn->_nullable == false && xmlColumn->_default.empty() == false)
		{
			// SET NOCOUNT ON : ���� ������ �� ��������� �� ������� ����
			_updateQueries[UpdateStep::AddColumn].push_back(DBModel::Helpers::Format(L"SET NOCOUNT ON; UPDATE [dbo].[%s] SET [%s] = %s WHERE [%s] IS NULL",
				dbTable->_name.c_str(), xmlColumn->_name.c_str(), xmlColumn->_default.c_str(), xmlColumn->_name.c_str()));
		}

		if (xmlColumn->_nullable == false)
		{
			_updateQueries[UpdateStep::AddColumn].push_back(DBModel::Helpers::Format(L"ALTER TABLE [dbo].[%s] ALTER COLUMN %s",
				dbTable->_name.c_str(), xmlColumn->CreateText().c_str()));
		}

		if (xmlColumn->_default.empty() == false)
		{
			_updateQueries[UpdateStep::AddColumn].push_back(DBModel::Helpers::Format(L"ALTER TABLE [dbo].[%s] ADD CONSTRAINT [DF_%s_%s] DEFAULT (%s) FOR [%s]",
				dbTable->_name.c_str(), dbTable->_name.c_str(), xmlColumn->_name.c_str(), xmlColumn->_default.c_str(), xmlColumn->_name.c_str()));
		}
	}

	// XML�� �ִ� �ε��� ����� ���� �´�.
	map<wstring, CIndex*> xmlIndexMap;
	for (CIndex*& xmlIndex : xmlTable->_indexes)
	{
		xmlIndexMap[xmlIndex->GetUniqueName()] = xmlIndex;
	}

	// DB�� �ִ� ���̺� �ε������� ���鼭 XML�� ���ǵǾ� �ִ� �ε������ ���Ѵ�.
	// ���� �߰�, ����, ������ �ε����� �ִ��� ���캸�� �κ�
	for (CIndex*& dbIndex : dbTable->_indexes)
	{
		// xmlIndexMap���� DBIndex�� name�� ������ Index�� ã�´�.
		// �ٽ� ����, DataBase�� �ִ� Index�� XML�� �ִ��� �Ǻ�
		auto findIndex = xmlIndexMap.find(dbIndex->GetUniqueName());

		// Index�� XmlIndex�� �ְ�		
		if (findIndex != xmlIndexMap.end()
			// _dependentIndexes�� �ش� �ε����� ���� ��� 
			&& _dependentIndexes.find(dbIndex->GetUniqueName()) == _dependentIndexes.end())
		{
			// Index�� �����´�.
			CIndex* xmlIndex = findIndex->second;
			// ó�� �Ϸ�� Index�� �����Ѵ�. -> DB���� ���� �Ƚ�Ŵ
			xmlIndexMap.erase(findIndex);
		}
		else
		{
			// �� ã���� ��� ��, XML�� ���� ��� ( ���� �ؾ��� )
			G_Logger->WriteStdOut(en_Color::YELLOW, L"Dropping Index : [%s] [%s] %s %s\n", dbTable->_name.c_str(), dbIndex->_name.c_str(), dbIndex->GetKeyText().c_str(), dbIndex->GetTypeText().c_str());
			if (dbIndex->_primaryKey || dbIndex->_uniqueConstraint)
			{
				// ������ Index�� _PrimaryKey or UniqueKey���
				// ������׸� �����ش�.
				_updateQueries[UpdateStep::DropIndex].push_back(DBModel::Helpers::Format(L"ALTER TABLE [dbo].[%s] DROP CONSTRAINT [%s]", dbTable->_name.c_str(), dbIndex->_name.c_str()));
			}
			else
			{
				// ������ Index�� _PrimaryKey �� UniqueKey�� �ƴϸ�
				// Index�ɼ��� �����Ѵ�.
				_updateQueries[UpdateStep::DropIndex].push_back(DBModel::Helpers::Format(L"DROP INDEX [%s] ON [dbo].[%s]", dbIndex->_name.c_str(), dbTable->_name.c_str()));
			}
		}
	}

	// �ʿ��� ���ŵ��� ���� XML �ε��� ���Ǹ� ���� �߰�
	// -> DataBase���� ���� XMl�� �ִ� �׸���� �߰�
	for (auto& mapIt : xmlIndexMap)
	{
		// �߰��� index�� �����´�.
		CIndex* xmlIndex = mapIt.second;
		G_Logger->WriteStdOut(en_Color::YELLOW, L"Creating Index : [%s] %s %s [%s]\n", dbTable->_name.c_str(), xmlIndex->GetKeyText().c_str(), xmlIndex->GetTypeText().c_str(), xmlIndex->GetUniqueName().c_str());
		if (xmlIndex->_primaryKey || xmlIndex->_uniqueConstraint)
		{
			// �߰��� �ε����� primaryKey or unique�� ��� �ش� Index�� �������ִ� ������ ���� �� �����Ѵ�.
			_updateQueries[UpdateStep::CreateIndex].push_back(DBModel::Helpers::Format(L"ALTER TABLE [dbo].[%s] ADD CONSTRAINT [%s] %s %s (%s)",
				dbTable->_name.c_str(), xmlIndex->CreateName(dbTable->_name).c_str(), xmlIndex->GetKeyText().c_str(), xmlIndex->GetTypeText().c_str(), xmlIndex->CreateColumnsText().c_str()));
		}
		else 
		{
			// �� �ܿ��� Index�� �����ϴ� ������ ���� �� �����Ѵ�.
			_updateQueries[UpdateStep::CreateIndex].push_back(DBModel::Helpers::Format(L"CREATE %s INDEX [%s] ON [dbo].[%s] (%s)",
				xmlIndex->GetTypeText(), xmlIndex->CreateName(dbTable->_name).c_str(), dbTable->_name.c_str(), xmlIndex->CreateColumnsText().c_str()));
		}
	}
}

void DBSynchronizer::CompareColumns(CTable* dbTable, CColumn* dbColumn, CColumn* xmlColumn)
{
	uint8 flag = 0;

	// DBColumn�� Ÿ�԰� XMLColumn�� Ÿ���� �ٸ����
	// ��, Ÿ���� �����Ǿ��ٸ�
	if (dbColumn->_type != xmlColumn->_type)
	{
		flag |= ColumnFlag::Type;
	}

	// DBColumn�� ���̿� XMLColumn�� ���̰� �ٸ��� �� ���� 0 ���� Ŭ ���
	// ��, MaxLength�� �����Ǿ��ٸ�
	if (dbColumn->_maxLength != xmlColumn->_maxLength && xmlColumn->_maxLength > 0)
	{
		flag |= ColumnFlag::Length;
	}

	// DBColumn�� _nullable�� XMLColumn�� _nullable�� �ٸ����
	// ��, _nullable�� �����Ǿ��ٸ�
	if (dbColumn->_nullable != xmlColumn->_nullable)
	{
		flag |= ColumnFlag::Nullable;
	}

	// DBColumn�� _identity�� XMLColumn�� _identity�� �ٸ��ų�
	// DBColumn�� _identity�� ���� �ְ�, DBColumn�� _incrementValue�� XMLColumn�� _incrementValue�� �ٸ���
	// _incrementValue�� �ڵ����� �����ϴ� ���� �ǹ���
	if (dbColumn->_identity != xmlColumn->_identity || (dbColumn->_identity && dbColumn->_incrementValue != xmlColumn->_incrementValue))
	{
		flag |= ColumnFlag::Identity;
	}

	// DBColumn�� _default�� XMLColumn�� _default�� �ٸ����
	// ��, _default���� �����Ǿ��ٸ�
	if (dbColumn->_default != xmlColumn->_default)
	{
		flag |= ColumnFlag::Default;
	}

	if (flag)
	{
		G_Logger->WriteStdOut(en_Color::YELLOW, L"Updating Column [%s] : (%s) -> (%s)\n", dbTable->_name.c_str(), dbColumn->CreateText().c_str(), xmlColumn->CreateText().c_str());
	}
	else
	{
		return;
	}

	// ������ �ε����� ������ ���߿� �����ϱ� ���� ����Ѵ�.
	if (flag & (ColumnFlag::Type | ColumnFlag::Length | ColumnFlag::Nullable))
	{
		for (CIndex*& dbIndex : dbTable->_indexes)
		{
			if (dbIndex->DependsOn(dbColumn->_name))
			{
				_dependentIndexes.insert(dbIndex->GetUniqueName());
			}

			flag |= ColumnFlag::CheckDependency;
		}
	}

	// �⺻���� �����Ǿ��� ���
	if (flag & ColumnFlag::CheckDependency)
	{
		if (dbColumn->_defaultConstraintName.empty() == false)
		{
			_updateQueries[UpdateStep::AlterColumn].push_back(DBModel::Helpers::Format(
				L"ALTER TABLE [dbo].[%s] DROP CONSTRAINT [%s]",
				dbTable->_name.c_str(),
				dbColumn->_defaultConstraintName.c_str()));
		}
	}

	DBModel::CColumn newColumn = *dbColumn;
	newColumn._default = L"";
	newColumn._type = xmlColumn->_type;
	newColumn._maxLength = xmlColumn->_maxLength;
	newColumn._typeText = xmlColumn->_typeText;
	newColumn._seedValue = xmlColumn->_seedValue;
	newColumn._identity = xmlColumn->_identity;
	newColumn._incrementValue = xmlColumn->_incrementValue;

	if (flag & (ColumnFlag::Type | ColumnFlag::Length | ColumnFlag::Identity))
	{
		_updateQueries[UpdateStep::AlterColumn].push_back(DBModel::Helpers::Format(
			L"ALTER TABLE [dbo].[%s] ALTER COLUMN %s",
			dbTable->_name.c_str(),
			newColumn.CreateText().c_str()));
	}

	//if (flag & ColumnFlag::Identity)
	//{
	//	_updateQueries[UpdateStep::DropColumn].push_back(
	//		DBModel::Helpers::Format(L"ALTER TABLE [dbo].[%s] DROP COLUMN [%s]",
	//			dbTable->_name.c_str(), newColumn._name.c_str()));
	//	_updateQueries[UpdateStep::AddColumn].push_back(DBModel::Helpers::Format(L"ALTER TABLE [dbo].[%s] ADD %s %s",
	//		dbTable->_name.c_str(), xmlColumn->_name.c_str(), xmlColumn->_typeText.c_str()));	
	//}

	newColumn._nullable = xmlColumn->_nullable;
	// Nullable�� ������ �־��ٸ�
	if (flag & ColumnFlag::Nullable)
	{
		if (xmlColumn->_default.empty() == false)
		{
			_updateQueries[UpdateStep::AlterColumn].push_back(DBModel::Helpers::Format(
				L"SET NOCOUNT ON; UPDATE [dbo].[%s] SET [%s] = %s WHERE [%s] IS NULL",
				dbTable->_name.c_str(),
				xmlColumn->_name.c_str(),
				xmlColumn->_name.c_str(),
				xmlColumn->_name.c_str()));
		}

		_updateQueries[UpdateStep::AlterColumn].push_back(DBModel::Helpers::Format(
			L"ALTER TABLE [dbo].[%s] ALTER COLUMN %s",
			dbTable->_name.c_str(),
			newColumn.CreateText().c_str()));
	}

	// Default�� ������ �־��ٸ�
	if (flag & ColumnFlag::Default)
	{
		if (dbColumn->_defaultConstraintName.empty() == false)
		{
			_updateQueries[UpdateStep::AlterColumn].push_back(DBModel::Helpers::Format(
				L"ALTER TABLE [dbo].[%s] ADD CONSTRAINT [%s] DEFAULT (%s) FOR [%s]",
				dbTable->_name.c_str(),
				DBModel::Helpers::Format(L"DF_%s_%s", dbTable->_name.c_str(), dbColumn->_name.c_str()).c_str(),
				dbColumn->_default.c_str(), dbColumn->_name.c_str()));
		}
	}
}

void DBSynchronizer::CompareStoredProcedures()
{
	// XML�� �ִ� ���ν��� ����� ���� �´�.
	map<wstring, CProcedure*> xmlProceduresMap;
	for (CProcedure* xmlProcedure : _xmlProcedures)
	{
		xmlProceduresMap[xmlProcedure->_name] = xmlProcedure;
	}		

	// DB�� �����ϴ� ���̺� ���ν������� ���鼭 XML�� ���ǵ� ���ν������ ���Ѵ�.
	// �ٽ� ���� XML�� ���ǵǾ� �ִ� ���ν����� DB�� �ִ��� Ȯ���ϰ� ������ �߰��Ѵ�.
	for (CProcedure* dbProcedure : _dbProcedures)
	{
		// XML ���ν��� ��Ͽ� DB ���ν����� ã��
		auto findProcedure = xmlProceduresMap.find(dbProcedure->_name);
		// ã������
		if (findProcedure != xmlProceduresMap.end())
		{
			// ���ν����� ��������
			CProcedure* xmlProcedure = findProcedure->second;
			// CREATE PROCEDURE ������ ����
			wstring xmlBody = xmlProcedure->GenerateCreateQuery();
			// DB�� ������ �ִ� ���ν��� ���� �κа� XML�� ���ǵǾ� �ִ� ���ν��� ���� �κ��� ���Ѵ�.
			if (DBModel::Helpers::RemoveWhiteSpace(dbProcedure->_fullBody) != DBModel::Helpers::RemoveWhiteSpace(xmlBody))
			{
				// ������ �ٸ���
				// ���ν��� ������ ������Ʈ ���شٰ� �����
				G_Logger->WriteStdOut(en_Color::YELLOW, L"Updating Procedure : %s\n", dbProcedure->_name.c_str());
				// XML ���ν��� �������� ������Ʈ ���ִ� ������ ������ �����Ѵ�.
				_updateQueries[UpdateStep::StoredProcecure].push_back(xmlProcedure->GenerateAlterQuery());
			}

			// ó�� �Ϸ�� ���ν��� ����
			xmlProceduresMap.erase(findProcedure);
		}
	}

	// �ʿ��� ���ŵ��� ���� XML ���ν����� ������ ���� �߰��� ���ν����̹Ƿ� �߰��Ѵ�.
	for (auto& mapIt : xmlProceduresMap)
	{
		G_Logger->WriteStdOut(en_Color::YELLOW, L"Updating Procedure : %s\n", mapIt.first.c_str());
		_updateQueries[UpdateStep::StoredProcecure].push_back(mapIt.second->GenerateCreateQuery());
	}
}