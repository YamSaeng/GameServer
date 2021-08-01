#include "pch.h"
#include "DBSynchronizer.h"
#include "DBBind.h"
#include "XmlParser.h"
#include <regex>

//---------------------------------
// DB에 저장되어 있는 정보 긁어오기
//---------------------------------
namespace SP
{
	// DB에 있는 테이블과 컬럼 목록을 모두 긁어오기
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

	// DB 테이블과 컬럼을 바인딩 시켜서 메모리로 가져오기
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

	// 인덱스 모두 가져오기
	const WCHAR* QIndexes =
		L"	SELECT i.object_id, i.name as indexName, i.index_id, i.type, i.is_primary_key,"
		"		i.is_unique_constraint, ic.column_id, COL_NAME(ic.object_id, ic.column_id) as columnName"
		"	FROM sys.indexes AS i"
		"	JOIN sys.index_columns AS ic"
		"		ON i.object_id = ic.object_id AND i.index_id = ic.index_id"
		"	WHERE i.type > 0 AND i.object_id IN(SELECT object_id FROM sys.tables WHERE type = 'U')"
		"	ORDER BY i.object_id ASC, i.index_id ASC;";

	// 인덱스 목록 메모리로 가져오기
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

	// 저장되어 있는 프로시저를 모두 가져온다.
	const WCHAR* QStoredProcedures =
		L"	SELECT name, OBJECT_DEFINITION(object_id) AS body FROM sys.procedures;";

	// 프로시저 목록 메모리로 가져오기
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
	// xml파일에 있는 데이터들을 읽어서 메모리에 저장
	ParseXmlDB(path);

	// DB에 기록되어 있는 테이블, 인덱스, 프로시져 정보를 긁어와서 메모리에 저장
	GatherDBTables();
	GatherDBIndexes();
	GatherDBStoredProcedures();

	// xml파일의 DB정보와 DB에서 읽어들인 정보를 비교해서 
	// 누락된 정보 or 추가된 정보가 있으면 DB를 업데이트 한다. xml파일이 최신 DB정보	
	CompareDBModel(); // 이곳에서 업데이트 할 항목을 예약하고
	ExecuteUpdateQueries(); // 여기에서 업데이트를 실행한다.

	return true;
}

void DBSynchronizer::ParseXmlDB(const WCHAR* path)
{
	CXmlNode root;
	CXmlParser parser;
	ASSERT_CRASH(parser.ParseFromFile(path, OUT root));

	// <Table>들을 찾아서 Tables에 저장
	vector<CXmlNode> tables = root.FindChildren(L"Table");
	for (CXmlNode& table : tables)
	{
		CTable* Table = new CTable();

		// <Table>의 name Attribute(속성) 가져와서 저장해둠
		Table->_name = table.GetStringAttr(L"name");

		// <Table>의 <Column>들을 찾아서 Columns에 저장
		vector<CXmlNode> columns = table.FindChildren(L"Column");
		for (CXmlNode& ColumnNode : columns)
		{
			CColumn* Column = new CColumn();

			// <Column>의 name, typetext, type, nuillable 속성을 가져와서 저장해둠
			Column->_name = ColumnNode.GetStringAttr(L"name");
			Column->_typeText = ColumnNode.GetStringAttr(L"type");
			Column->_type = DBModel::Helpers::WStringToDataType(Column->_typeText.c_str(), OUT Column->_maxLength);
			ASSERT_CRASH(Column->_type != DBModel::DataType::None);
			Column->_nullable = !ColumnNode.GetBoolAttr(L"notnull", false);

			// <Column>의 Identity 속성을 가져온다.
			const WCHAR* identityStr = ColumnNode.GetStringAttr(L"identity");
			if (::wcslen(identityStr) > 0)
			{
				// 정규표현식 설정 (숫자,숫자)
				std::wregex pt(L"(\\d+),(\\d+)");
				std::wcmatch match;

				// 정규표현식(숫자,숫자)으로 패턴 검사해서 match에 저장
				ASSERT_CRASH(std::regex_match(identityStr, OUT match, pt));
				Column->_identity = true;
				Column->_seedValue = _wtoi(match[1].str().c_str());
				Column->_incrementValue = _wtoi(match[2].str().c_str());
			}

			// <Column>의 default속성을 가져와서 저장
			Column->_default = ColumnNode.GetStringAttr(L"default");
			// 완성한 컬럼을 저장
			Table->_columns.push_back(Column);
		}

		// <Table>의 <Index>들을 찾아서 Indexes에 저장
		vector<CXmlNode> indexes = table.FindChildren(L"Index");
		for (CXmlNode& IndexNode : indexes)
		{
			CIndex* i = new CIndex();

			// 타입 읽어옴
			const WCHAR* typeStr = IndexNode.GetStringAttr(L"type");

			// 타입이 clustered인지 nonclustered인지 확인
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

			// <PrimaryKey>가 있는지 확인
			i->_primaryKey = IndexNode.FindChild(L"PrimaryKey").IsValid();
			// <UniqueKey>가 있는지 확인
			i->_uniqueConstraint = IndexNode.FindChild(L"UniqueKey").IsValid();

			// <Table> - <Index> - <Column>들을 찾아서 Columns에 저장
			vector<CXmlNode> columns = IndexNode.FindChildren(L"Column");
			// 읽어들인 Column 순환하면서
			for (CXmlNode& column : columns)
			{
				// 컬럼 이름을 가지고 옴
				const WCHAR* nameStr = column.GetStringAttr(L"name");
				// 컬럼 생성
				CColumn* c = Table->FindColumn(nameStr);
				ASSERT_CRASH(c != nullptr);
				// Index의 columns에 저장
				i->_columns.push_back(c);
			}

			// 완성된 Index를 저장
			Table->_indexes.push_back(i);
		}

		// 최종적으로 완성된 T (테이블) 저장
		_xmlTables.push_back(Table);
	}

	// <Procedure>들을 찾아서 가지고옴
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

	// 제거할 테이블 목록
	// CompareTable을 통해 DB와 xmlDB를 비교후 삭제할 테이블이 있을경우 
	// 제거할 테이블 목록에 삭제할 테이블이 있어야 최종적으로 해당 테이블을 삭제한다.
	vector<CXmlNode> removedTables = root.FindChildren(L"RemovedTable");
	for (CXmlNode& removedTable : removedTables)
	{
		_xmlRemovedTables.insert(removedTable.GetStringAttr(L"name"));
	}
}

// DB에 기록되어 있는 테이블, 인덱스, 프로시져 정보를 긁어와서 메모리에 저장
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

	// 테이블에 기록되어 있는 정보를 DB로 부터 가져온다.
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

	// DB에 질의
	if (getDBTables.Execute() == false)
	{
		return false;
	}

	// 데이터 가져오기
	while (getDBTables.Fetch())
	{
		CTable* table;

		auto findTable = std::find_if(_dbTables.begin(), _dbTables.end(), [=](const CTable* table) { return table->_objectId == objectId; });
		if (findTable == _dbTables.end())
		{
			// 중복되는게 없으니 새로 생성
			table = new CTable();
			table->_objectId = objectId;
			table->_name = tableName;
			_dbTables.push_back(table);
		}
		else
		{
			// 중복되는게 있으니 있던거 꺼내서 가지고 옴
			table = *findTable;
		}

		// DB에서 읽어들인 컬럼 정보 셋팅
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

		// 컬럼의 디폴트 값이 0보다 클경우
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

// DB에 있는 Index정보 긁어오기
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

	// DB에 기록되어 있는 Index 정보를 긁어온다.
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

		// 찾은 테이블의 인덱스들 긁어옴
		vector<CIndex*>& indexes = (*findTable)->_indexes;
		auto findIndex = std::find_if(indexes.begin(), indexes.end(), [indexId](const CIndex* index) { return index->_indexId == indexId; });
		if (findIndex == indexes.end())
		{
			// 중복되는게 없으니 새로 생성
			CIndex* index = new CIndex();

			index->_name = indexName;
			index->_indexId = indexId;
			index->_type = static_cast<DBModel::IndexType>(indexType);
			index->_primaryKey = isPrimaryKey;
			index->_uniqueConstraint = isUniqueConstraint;

			indexes.push_back(index);
			findIndex = indexes.end() - 1;
		}

		// 인덱스가 걸린 column 찾아서 매핑해준다.
		vector<CColumn*>& columns = (*findTable)->_columns;
		auto findColumn = std::find_if(columns.begin(), columns.end(), [columnId](const CColumn* column) { return column->_columnId == columnId; });
		ASSERT_CRASH(findColumn != columns.end());
		(*findIndex)->_columns.push_back(*findColumn);
	}

	return true;
}

// DB에 있는 Procedure 정보 긁어오기
bool DBSynchronizer::GatherDBStoredProcedures()
{
	WCHAR name[101] = { 0 };
	vector<WCHAR> body(PROCEDURE_MAX_LEN);

	// DB에서 저장되어 있는 Procedure 정보 긁어오기
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

// 보관중인 xml기반 DB 내용과 직접 DB에서 읽어들인 내용을 비교
void DBSynchronizer::CompareDBModel()
{
	// 업데이트 목록 초기화.
	_dependentIndexes.clear();
	for (vector<wstring>& queries : _updateQueries)
	{
		queries.clear();
	}

	// XML에 있는 목록을 우선 갖고 온다.
	map<wstring, CTable*> xmlTableMap;
	for (CTable*& xmlTable : _xmlTables)
	{
		xmlTableMap[xmlTable->_name] = xmlTable;
	}

	// DB에 실존하는 테이블들을 돌면서 XML에 정의된 테이블들과 비교한다.
	// 새로 수정, 추가 삭제된 테이블이 있는지 확인 하는 부분
	for (CTable*& dbTable : _dbTables)
	{
		// XmlTableMap에서 DBTable의 name을 가지고 Table을 찾는다.
		// 다시 말해, DataBase에 있는 테이블이 XML에 있는지 판별
		auto findTable = xmlTableMap.find(dbTable->_name);
		// 찾았을 경우
		// 즉, XML에만 있는 경우
		if (findTable != xmlTableMap.end())
		{
			// 테이블을 가지고 옴
			CTable* xmlTable = findTable->second;
			// 테이블을 비교한다. (새로 수정, 추가, 삭제된 컬럼이 있는지 확인)
			CompareTables(dbTable, xmlTable);
			// 처리 완료된 테이블 제거 
			xmlTableMap.erase(findTable);
		}
		else
		{
			// 못 찾았을 경우
			// 즉, XML에 없는 경우 -> 삭제 해야함
			// 만약 삭제 테이블 목록에 삭제하려는 테이블이 있으면
			if (_xmlRemovedTables.find(dbTable->_name) != _xmlRemovedTables.end())
			{
				// 콘솔 출력하고					
				G_Logger->WriteStdOut(en_Color::YELLOW, L"Removing Table : [dbo].[%s]\n", dbTable->_name.c_str());
				// DROP TABLE 쿼리문 조합후 예약
				_updateQueries[UpdateStep::DropTable].push_back(DBModel::Helpers::Format(L"DROP TABLE [dbo].[%s]", dbTable->_name.c_str()));
			}
		}
	}

	// 맵에서 제거되지 않은 XML 테이블 정의를 새로 추가
	// -> DataBase에는 없고 XMl에 있는 항목들을 추가
	for (auto& mapIt : xmlTableMap)
	{
		CTable*& xmlTable = mapIt.second;

		// 컬럼 이름 조합
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

		// CREATE TABLE 콘솔 출력해서 알려줌
		G_Logger->WriteStdOut(en_Color::YELLOW, L"Creating Table : [dbo].[%s]\n", xmlTable->_name.c_str());
		// CREATE TABLE 쿼리문 조합후 예약
		_updateQueries[UpdateStep::CreateTable].push_back(DBModel::Helpers::Format(L"CREATE TABLE [dbo].[%s] (%s)", xmlTable->_name.c_str(), columnsStr.c_str()));

		// XmlTable에 있는 속성 돌면서
		for (CColumn*& xmlColumn : xmlTable->_columns)
		{
			// Defualt 체크 
			// 비워져 있으면 넘김
			if (xmlColumn->_default.empty())
			{
				continue;
			}

			// Default가 안비워져 있으면 Default에 관한 쿼리문 작성후 예약
			_updateQueries[UpdateStep::DefaultConstraint].push_back(DBModel::Helpers::Format(L"ALTER TABLE [dbo].[%s] ADD CONSTRAINT [%s] DEFAULT (%s) FOR [%s]",
				xmlTable->_name.c_str(),
				DBModel::Helpers::Format(L"DF_%s_%s", xmlTable->_name.c_str(), xmlColumn->_name.c_str()).c_str(),
				xmlColumn->_default.c_str(),
				xmlColumn->_name.c_str()));
		}

		// XMLTable에 있는 인덱스를 돌면서
		for (CIndex*& xmlIndex : xmlTable->_indexes)
		{
			// INDEX 생성 콘솔 출력해서 알려줌
			G_Logger->WriteStdOut(en_Color::YELLOW, L"Creating Index : [%s] %s %s [%s]\n", xmlTable->_name.c_str(), xmlIndex->GetKeyText().c_str(), xmlIndex->GetTypeText().c_str(), xmlIndex->GetUniqueName().c_str());
			// 인덱스에 PrimaryKey or UniqueKey가 걸려있으면
			if (xmlIndex->_primaryKey || xmlIndex->_uniqueConstraint)
			{
				// 기존에 생성되어 있었던 속성에 PrimaryKey or Unique 제약조건을 걸어서 속성을 변경시켜주는 쿼리문

				// PRIMARY KEY
				// ALTER TABLE [dbo].[Account] ADD CONSTRAINT [IX_Account_accountID] PRIMARY KEY CLUSTERED ([accountID])
				// UNIQUE
				// ALTER TABLE [dbo].[Account] ADD CONSTRAINT [IX_Account_testId_testId2] UNIQUE NONCLUSTERED ([testId],[testId2])
				_updateQueries[UpdateStep::CreateIndex].push_back(DBModel::Helpers::Format(
					L"ALTER TABLE [dbo].[%s] ADD CONSTRAINT [%s] %s %s (%s)",
					xmlTable->_name.c_str(), // 테이블 이름
					xmlIndex->CreateName(xmlTable->_name).c_str(), // 추가할 인덱스키의 이름 생성
					xmlIndex->GetKeyText().c_str(),  // PRIMARYKEY 인지 UNIQUEKEY 인지
					xmlIndex->GetTypeText().c_str(), // 추가한 키가 CLUSTERED인지 NONCLUSTERED 인지
					xmlIndex->CreateColumnsText().c_str())); // PRIMARYKEY or UNIQUEKEY로 추가할 컬럼의 이름 
			}
			else
			{
				// 인덱스에 PrimaryKey or UniqueKey가 걸려있지 않으면

				// PrimaryKey or UniqueKey 를 생성해서 추가하는 쿼리문 조합해서 예약
				// CREATE CLUSTERED or NONCLUSTERED [IX_테이블명_이름] [dbo].[테이블명] (컬럼이름)
				_updateQueries[UpdateStep::CreateIndex].push_back(DBModel::Helpers::Format(
					L"CREATE %s INDEX [%s] ON [dbo].[%s] (%s)",
					xmlIndex->GetTypeText().c_str(), // CLUSTERED 인지 NONCLUSTERED 인지
					xmlIndex->CreateName(xmlTable->_name).c_str(), // 추가할 인덱스키의 이름 
					xmlTable->_name.c_str(), // 테이블 이름
					xmlIndex->CreateColumnsText().c_str())); // 인덱스로 추가할 컬럼의 이름
			}
		}
	}

	// DB에 있는 Procedure와 xml Procedure 비교
	CompareStoredProcedures();
}

void DBSynchronizer::ExecuteUpdateQueries()
{
	// 저장 되어 있는 쿼리 실행
	for (int32 step = 0; step < UpdateStep::Max; step++)
	{
		for (wstring& query : _updateQueries[step])
		{
			_dbConn.UnBind();
			ASSERT_CRASH(_dbConn.Execute(query.c_str()));
		}
	}
}

// DB에 저장되어 있던 테이블과 XML에 정의되어 있던 테이블을 비교한다.
void DBSynchronizer::CompareTables(CTable* dbTable, CTable* xmlTable)
{
	// XML에 있는 컬럼 목록을 갖고 온다.
	map<wstring, CColumn*> xmlColumnMap;
	for (CColumn*& xmlColumn : xmlTable->_columns)
	{
		xmlColumnMap[xmlColumn->_name] = xmlColumn;
	}

	// DB에 실존하는 테이블 컬럼들을 돌면서 XML에 정의된 컬럼들과 비교한다.
	// 새로 추가된, 수정, 삭제된 컬럼이 있는지 확인하는 부분
	for (CColumn*& DBColumn : dbTable->_columns)
	{
		// XmlColumnMap에서 DBColumn의 name을 가지고 Column을 찾는다.
		// 다시 말해, DataBase에 있는 Column이 XML에 있는지 판별
		auto findColumn = xmlColumnMap.find(DBColumn->_name);
		// 찾았을 경우 즉, XML에만 있는 경우
		if (findColumn != xmlColumnMap.end())
		{
			// 컬럼을 가져오고
			CColumn*& xmlColumn = findColumn->second;
			// 컬럼 비교
			CompareColumns(dbTable, DBColumn, xmlColumn);
			// 처리 완료된 컬럼 제거
			xmlColumnMap.erase(findColumn);
		}
		else
		{
			// 못 찾았을 경우 즉, XML에 없는 경우 ( 삭제 해야함 )
			G_Logger->WriteStdOut(en_Color::YELLOW, L"Dropping Column : [%s].[%s]\n", dbTable->_name.c_str(), DBColumn->_name.c_str());
			// 제약사항이 잇는지 확인
			if (DBColumn->_defaultConstraintName.empty() == false)
			{
				//제약 사항 있으면 제약 사항 제거하는 쿼리문 작성후 예약
				_updateQueries[UpdateStep::DropColumn].push_back(DBModel::Helpers::Format(L"ALTER TABLE [dbo].[%s] DROP CONSTRAINT [%s]", dbTable->_name.c_str(), DBColumn->_defaultConstraintName.c_str()));
			}

			// 쿼리문 생성 후 예약
			_updateQueries[UpdateStep::DropColumn].push_back(DBModel::Helpers::Format(L"ALTER TABLE [dbo].[%s] DROP COLUMN [%s]", dbTable->_name.c_str(), DBColumn->_name.c_str()));
		}
	}

	// 맵에서 제거되지 않은 XML 컬럼을 새로 추가
	// -> DataBase에는 없고 XMl에 있는 항목들을 추가
	for (auto& NewColumnIt : xmlColumnMap)
	{
		CColumn*& xmlColumn = NewColumnIt.second;
		DBModel::CColumn newColumn = *xmlColumn;
		newColumn._nullable = true;

		// 콘솔로 Adding Column 항목 알려줌
		G_Logger->WriteStdOut(en_Color::YELLOW, L"Adding Column : [%s].[%s]\n", dbTable->_name.c_str(), xmlColumn->_name.c_str());
		// 컬럼 추가 쿼리문 조합 후 예약
		_updateQueries[UpdateStep::AddColumn].push_back(DBModel::Helpers::Format(L"ALTER TABLE [dbo].[%s] ADD %s %s",
			dbTable->_name.c_str(), xmlColumn->_name.c_str(), xmlColumn->_typeText.c_str()));

		if (xmlColumn->_nullable == false && xmlColumn->_default.empty() == false)
		{
			// SET NOCOUNT ON : 쿼리 수행결과 중 영향받은행 수 출력하지 않음
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

	// XML에 있는 인덱스 목록을 갖고 온다.
	map<wstring, CIndex*> xmlIndexMap;
	for (CIndex*& xmlIndex : xmlTable->_indexes)
	{
		xmlIndexMap[xmlIndex->GetUniqueName()] = xmlIndex;
	}

	// DB에 있는 테이블 인덱스들을 돌면서 XML에 정의되어 있는 인덱스들과 비교한다.
	// 새로 추가, 삭제, 수정된 인덱스가 있는지 살펴보는 부분
	for (CIndex*& dbIndex : dbTable->_indexes)
	{
		// xmlIndexMap에서 DBIndex의 name을 가지고 Index를 찾는다.
		// 다시 말해, DataBase에 있는 Index가 XML에 있는지 판별
		auto findIndex = xmlIndexMap.find(dbIndex->GetUniqueName());

		// Index가 XmlIndex에 있고		
		if (findIndex != xmlIndexMap.end()
			// _dependentIndexes에 해당 인덱스가 없을 경우 
			&& _dependentIndexes.find(dbIndex->GetUniqueName()) == _dependentIndexes.end())
		{
			// Index를 가져온다.
			CIndex* xmlIndex = findIndex->second;
			// 처리 완료된 Index를 제거한다. -> DB에서 삭제 안시킴
			xmlIndexMap.erase(findIndex);
		}
		else
		{
			// 못 찾았을 경우 즉, XML에 없는 경우 ( 삭제 해야함 )
			G_Logger->WriteStdOut(en_Color::YELLOW, L"Dropping Index : [%s] [%s] %s %s\n", dbTable->_name.c_str(), dbIndex->_name.c_str(), dbIndex->GetKeyText().c_str(), dbIndex->GetTypeText().c_str());
			if (dbIndex->_primaryKey || dbIndex->_uniqueConstraint)
			{
				// 삭제할 Index가 _PrimaryKey or UniqueKey라면
				// 제약사항만 없애준다.
				_updateQueries[UpdateStep::DropIndex].push_back(DBModel::Helpers::Format(L"ALTER TABLE [dbo].[%s] DROP CONSTRAINT [%s]", dbTable->_name.c_str(), dbIndex->_name.c_str()));
			}
			else
			{
				// 삭제할 Index가 _PrimaryKey 와 UniqueKey가 아니면
				// Index옵션을 제거한다.
				_updateQueries[UpdateStep::DropIndex].push_back(DBModel::Helpers::Format(L"DROP INDEX [%s] ON [dbo].[%s]", dbIndex->_name.c_str(), dbTable->_name.c_str()));
			}
		}
	}

	// 맵에서 제거되지 않은 XML 인덱스 정의를 새로 추가
	// -> DataBase에는 없고 XMl에 있는 항목들을 추가
	for (auto& mapIt : xmlIndexMap)
	{
		// 추가할 index를 가져온다.
		CIndex* xmlIndex = mapIt.second;
		G_Logger->WriteStdOut(en_Color::YELLOW, L"Creating Index : [%s] %s %s [%s]\n", dbTable->_name.c_str(), xmlIndex->GetKeyText().c_str(), xmlIndex->GetTypeText().c_str(), xmlIndex->GetUniqueName().c_str());
		if (xmlIndex->_primaryKey || xmlIndex->_uniqueConstraint)
		{
			// 추가할 인덱스가 primaryKey or unique일 경우 해당 Index를 수정해주는 쿼리문 조합 후 예약한다.
			_updateQueries[UpdateStep::CreateIndex].push_back(DBModel::Helpers::Format(L"ALTER TABLE [dbo].[%s] ADD CONSTRAINT [%s] %s %s (%s)",
				dbTable->_name.c_str(), xmlIndex->CreateName(dbTable->_name).c_str(), xmlIndex->GetKeyText().c_str(), xmlIndex->GetTypeText().c_str(), xmlIndex->CreateColumnsText().c_str()));
		}
		else 
		{
			// 그 외에는 Index를 생성하는 쿼리문 조합 후 예약한다.
			_updateQueries[UpdateStep::CreateIndex].push_back(DBModel::Helpers::Format(L"CREATE %s INDEX [%s] ON [dbo].[%s] (%s)",
				xmlIndex->GetTypeText(), xmlIndex->CreateName(dbTable->_name).c_str(), dbTable->_name.c_str(), xmlIndex->CreateColumnsText().c_str()));
		}
	}
}

void DBSynchronizer::CompareColumns(CTable* dbTable, CColumn* dbColumn, CColumn* xmlColumn)
{
	uint8 flag = 0;

	// DBColumn의 타입과 XMLColumn의 타입이 다를경우
	// 즉, 타입이 수정되었다면
	if (dbColumn->_type != xmlColumn->_type)
	{
		flag |= ColumnFlag::Type;
	}

	// DBColumn의 길이와 XMLColumn의 길이가 다르고 그 값이 0 보다 클 경우
	// 즉, MaxLength가 수정되었다면
	if (dbColumn->_maxLength != xmlColumn->_maxLength && xmlColumn->_maxLength > 0)
	{
		flag |= ColumnFlag::Length;
	}

	// DBColumn의 _nullable과 XMLColumn의 _nullable이 다를경우
	// 즉, _nullable이 수정되었다면
	if (dbColumn->_nullable != xmlColumn->_nullable)
	{
		flag |= ColumnFlag::Nullable;
	}

	// DBColumn의 _identity과 XMLColumn의 _identity이 다르거나
	// DBColumn의 _identity는 켜져 있고, DBColumn의 _incrementValue과 XMLColumn의 _incrementValue이 다를때
	// _incrementValue은 자동으로 증가하는 값을 의미함
	if (dbColumn->_identity != xmlColumn->_identity || (dbColumn->_identity && dbColumn->_incrementValue != xmlColumn->_incrementValue))
	{
		flag |= ColumnFlag::Identity;
	}

	// DBColumn의 _default과 XMLColumn의 _default이 다를경우
	// 즉, _default값이 수정되었다면
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

	// 연관된 인덱스가 있으면 나중에 삭제하기 위해 기록한다.
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

	// 기본값이 수정되었을 경우
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
	// Nullable의 변경이 있엇다면
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

	// Default의 변경이 있었다면
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
	// XML에 있는 프로시저 목록을 갖고 온다.
	map<wstring, CProcedure*> xmlProceduresMap;
	for (CProcedure* xmlProcedure : _xmlProcedures)
	{
		xmlProceduresMap[xmlProcedure->_name] = xmlProcedure;
	}		

	// DB에 실존하는 테이블 프로시저들을 돌면서 XML에 정의된 프로시저들과 비교한다.
	// 다시 말해 XML에 정의되어 있는 프로시저가 DB에 있는지 확인하고 없으면 추가한다.
	for (CProcedure* dbProcedure : _dbProcedures)
	{
		// XML 프로시저 목록에 DB 프로시저를 찾음
		auto findProcedure = xmlProceduresMap.find(dbProcedure->_name);
		// 찾았으면
		if (findProcedure != xmlProceduresMap.end())
		{
			// 프로시저를 가져오고
			CProcedure* xmlProcedure = findProcedure->second;
			// CREATE PROCEDURE 쿼리문 생성
			wstring xmlBody = xmlProcedure->GenerateCreateQuery();
			// DB가 가지고 있는 프로시저 몸통 부분과 XML에 정의되어 있는 프로시저 몸통 부분을 비교한다.
			if (DBModel::Helpers::RemoveWhiteSpace(dbProcedure->_fullBody) != DBModel::Helpers::RemoveWhiteSpace(xmlBody))
			{
				// 내용이 다르면
				// 프로시저 내용을 업데이트 해준다고 출력후
				G_Logger->WriteStdOut(en_Color::YELLOW, L"Updating Procedure : %s\n", dbProcedure->_name.c_str());
				// XML 프로시저 기준으로 업데이트 해주는 쿼리를 생성후 예약한다.
				_updateQueries[UpdateStep::StoredProcecure].push_back(xmlProcedure->GenerateAlterQuery());
			}

			// 처리 완료된 프로시저 제거
			xmlProceduresMap.erase(findProcedure);
		}
	}

	// 맵에서 제거되지 않은 XML 프로시저가 있으면 새로 추가된 프로시저이므로 추가한다.
	for (auto& mapIt : xmlProceduresMap)
	{
		G_Logger->WriteStdOut(en_Color::YELLOW, L"Updating Procedure : %s\n", mapIt.first.c_str());
		_updateQueries[UpdateStep::StoredProcecure].push_back(mapIt.second->GenerateCreateQuery());
	}
}