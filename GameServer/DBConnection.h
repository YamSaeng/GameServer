#pragma once
#include <sql.h>
#include <sqlext.h>

enum
{
	WVARCHAR_MAX = 4000,
	BINARY_MAX = 8000
};

class CDBConnection
{
private:
	// SQL ���� �ڵ�
	SQLHDBC _DBConnectin = SQL_NULL_HANDLE;
	SQLHSTMT _Statement = SQL_NULL_HANDLE;
public:
	// �Է¹��� ConnectionString�� ���� DB�� ����
	bool Connect(SQLHENV Henv, const WCHAR* ConnectionString);
	// �ڵ� ����
	void Clear();

	// �Է¹��� ������ ����
	bool Execute(const WCHAR* Query);
	// ���� �Ϸ��� ������ �����ö� �ʿ��� �Լ�	
	bool Fetch();
	// �� ���� ��ȯ
	int32 GetRowCount();

	// ������ �޾ѵξ��� DB ��������� ����
	// (Statement�� �����ѰͰ� ����)
	void UnBind();

	// ���� �����Ҷ� �Ѱ��ִ� ���ڸ� ���ε� �Ҷ� ���
	bool BindParam(int32 ParamIndex, bool* Value, SQLLEN* Index);
	bool BindParam(int32 ParamIndex, float* Value, SQLLEN* Index);
	bool BindParam(int32 ParamIndex, double* Value, SQLLEN* Index);
	bool BindParam(int32 ParamIndex, int8* Value, SQLLEN* Index);
	bool BindParam(int32 ParamIndex, int16* Value, SQLLEN* Index);
	bool BindParam(int32 ParamIndex, int32* Value, SQLLEN* Index);
	bool BindParam(int32 ParamIndex, int64* Value, SQLLEN* Index);
	bool BindParam(int32 ParamIndex, TIMESTAMP_STRUCT* TimeValue, SQLLEN* Index);
	bool BindParam(int32 ParamIndex, const WCHAR* Str, SQLLEN* Index);
	bool BindParam(int32 ParamIndex, const char* Str, SQLLEN* Index);
	bool BindParam(int32 ParamIndex, const BYTE* Binary, int32 Size, SQLLEN* Index);

	// ���� ������ ���õ� �����͸� �޾ƿö� ����� �Լ�
	bool BindCol(int32 ColumnIndex, bool* Value, SQLLEN* Index);
	bool BindCol(int32 ColumnIndex, float* Value, SQLLEN* Index);
	bool BindCol(int32 ColumnIndex, double* Value, SQLLEN* Index);
	bool BindCol(int32 ColumnIndex, int8* Value, SQLLEN* Index);
	bool BindCol(int32 ColumnIndex, int16* Value, SQLLEN* Index);
	bool BindCol(int32 ColumnIndex, int32* Value, SQLLEN* Index);
	bool BindCol(int32 ColumnIndex, int64* Value, SQLLEN* Index);
	bool BindCol(int32 ColumnIndex, TIMESTAMP_STRUCT* Value, SQLLEN* Index);	
	bool BindCol(int32 ColumnIndex, WCHAR* Str, int32 Size, SQLLEN* Index);
	bool BindCol(int32 ColumnIndex, BYTE* Binary, int32 Size, SQLLEN* Index);

private:
	// SQL ������ �����Ҷ� ���ڸ� �Ѱ�����ϴ� ��찡 ���� ����µ�, SQL �������� _Statement�� ���� ���ڸ� �Ѱ��ٶ� 
	// ���õ� �Լ�	
	bool BindParam(SQLUSMALLINT ParamIndex, SQLUSMALLINT CType, SQLSMALLINT SqlType, SQLULEN Length, SQLPOINTER Ptr, SQLLEN* Index);
	// ���� ������ ���� ���õ� �����͸� �޾ƿö� ����� �Լ�
	bool BindCol(SQLUSMALLINT ColumnIndex, SQLSMALLINT CType, SQLULEN Length, SQLPOINTER Value, SQLLEN* Index);
	// �����߻��� ȣ��
	void DBError(SQLRETURN Ret);
};

