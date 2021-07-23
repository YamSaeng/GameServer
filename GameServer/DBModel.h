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
		wstring				_name;					// Column �̸�
		int32				_columnId = 0;			// DB
		DataType			_type = DataType::None; // Column�� Ÿ��
		wstring				_typeText;				// Type�� ���� �ؽ�Ʈ 
		int32				_maxLength = 0;			// Type�� nvarchar�� ��� �ִ����
		bool				_nullable = false;		// nullable ����
		bool				_identity = false;		// identity ���� identity�� ������ �����ʹ� �ڵ����� ���� �����Ѵ�.
		int64				_seedValue = 0;			// identity �ʱⰪ
		int64				_incrementValue = 0;	// identity�� �ڵ����� �����ϴ� ��
		wstring				_default;				// ���� default��
		wstring				_defaultConstraintName; // ���� ���� DB
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
		IndexType			_type = IndexType::NonClustered; // IndexType ����
		bool				_primaryKey = false;			 // �����̸Ӹ� Ű ����
		bool				_uniqueConstraint = false;		 // ����ũ Ű ����		
		vector<CColumn*>	_columns;						 // �ε����� �ɷ��� �ִ� �÷����� ��͵������� ���� ����
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
		wstring				_name;		   // ���̺� �̸�
		vector<CColumn*>	_columns;	   // ���̺� ���� �÷���
		vector<CIndex*>		_indexes;	   // ���̺� ���� �ε�����
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

	// DB ���ν��� ����
	class CProcedure
	{
	public:
		wstring				_name;		 // ���ν��� �̸�
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