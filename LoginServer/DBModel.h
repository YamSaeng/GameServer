#pragma once

namespace DBModel
{
	//--------------------
	// DataType
	//--------------------
	enum class DataType
	{
		None = 0,
		TinyInt = 48,
		SmallInt = 52,
		Int = 56,
		Real = 59,
		DateTime = 61,
		Float = 62,
		Bit = 104,
		Numeric = 108,
		BigInt = 127,
		VarBinary = 165,
		Varchar = 167,
		Binary = 173,
		NVarChar = 231,
	};

	//--------------------
	// Column 
	//--------------------
	class CColumn
	{
	public:
		wstring				_name;					// Column 이름
		int32				_columnId = 0;			// DB
		DataType			_type = DataType::None; // Column의 타입
		wstring				_typeText;				// Type에 관한 텍스트 
		int32				_maxLength = 0;			// Type이 nvarchar일 경우 최대길이
		bool				_nullable = false;		// nullable 여부
		bool				_identity = false;		// identity 여부 identity를 설정한 데이터는 자동으로 값이 증가한다.
		int64				_seedValue = 0;			// identity 초기값
		int64				_incrementValue = 0;	// identity가 자동으로 증가하는 값
		wstring				_default;				// 변수 default값
		wstring				_defaultConstraintName; // 제약 조건 DB
	public:
		wstring				CreateText();
	};

	//--------------------
	// Index 
	//--------------------
	enum class IndexType
	{
		Clustered = 1,
		NonClustered = 2
	};

	class CIndex
	{
	public:
		wstring				_name;							 // DB
		int32				_indexId = 0;					 // DB
		IndexType			_type = IndexType::NonClustered; // IndexType 여부
		bool				_primaryKey = false;			 // 프라이머리 키 여부
		bool				_uniqueConstraint = false;		 // 유니크 키 여부		
		vector<CColumn*>	_columns;						 // 인덱스가 걸려져 있는 컬럼들이 어떤것들인지에 대한 여부
	public:
		wstring				GetUniqueName();
		wstring				CreateName(const wstring& tableName);
		wstring				GetTypeText();
		wstring				GetKeyText();
		wstring				CreateColumnsText();
		bool				DependsOn(const wstring& columnName);
	};

	//--------------------
	// Table 
	//--------------------
	class CTable
	{
	public:
		int32				_objectId = 0; // DB
		wstring				_name;		   // 테이블 이름
		vector<CColumn*>	_columns;	   // 테이블에 속한 컬럼들
		vector<CIndex*>		_indexes;	   // 테이블에 속한 인덱스들
	public:
		CColumn* FindColumn(const wstring& columnName);
	};

	//--------------------
	// Procedure 
	//--------------------
	struct st_Param
	{
		wstring				_name;
		wstring				_type;
	};

	// DB 프로시저 정보
	class CProcedure
	{
	public:
		wstring				_name;		 // 프로시저 이름
		wstring				_fullBody;   // DB
		wstring				_body;		 // XML
		vector<st_Param>		_parameters; // XML

	public:
		wstring				GenerateCreateQuery();
		wstring				GenerateAlterQuery();
		wstring				GenerateParamWString();
	};

	class Helpers
	{
	public:
		static wstring		Format(const WCHAR* Format, ...);
		static wstring		DataTypeToWString(DataType Type);
		static wstring		RemoveWhiteSpace(const wstring& Str);
		static DataType		WStringToDataType(const WCHAR* Str, OUT int32& MaxLength);
	};
}