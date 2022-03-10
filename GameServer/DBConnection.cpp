#include "pch.h"
#include "DBConnection.h"

bool CDBConnection::Connect(SQLHENV Henv, const WCHAR* ConnectionString)
{
	if (SQLAllocHandle(SQL_HANDLE_DBC, Henv, &_DBConnectin) != SQL_SUCCESS)
	{
		return false;
	}

	WCHAR StringBuffer[MAX_PATH] = { 0 };
	wcscpy_s(StringBuffer, ConnectionString);

	WCHAR ResultString[MAX_PATH] = { 0 };
	SQLSMALLINT ResultStringLength = 0;

	SQLRETURN SQLRet = SQLDriverConnect(_DBConnectin,
		NULL,
		(SQLWCHAR*)(StringBuffer),
		_countof(StringBuffer),
		OUT(SQLWCHAR*)(ResultString),
		_countof(ResultString),
		OUT & ResultStringLength,
		SQL_DRIVER_NOPROMPT);

	if (SQLAllocHandle(SQL_HANDLE_STMT, _DBConnectin, &_Statement) != SQL_SUCCESS)
	{
		return false;
	}

	if (SQLRet == SQL_SUCCESS || SQLRet == SQL_SUCCESS_WITH_INFO)
	{
		return true;
	}
}

void CDBConnection::Clear()
{
	if (_DBConnectin != SQL_NULL_HANDLE)
	{
		SQLFreeHandle(SQL_HANDLE_DBC, _DBConnectin);
		_DBConnectin = SQL_NULL_HANDLE;
	}

	if (_Statement != SQL_NULL_HANDLE)
	{
		SQLFreeHandle(SQL_HANDLE_STMT, _Statement);
		_Statement = SQL_NULL_HANDLE;
	}
}

// 입력받은 쿼리문 실행
bool CDBConnection::Execute(const WCHAR* Query)
{
	SQLRETURN QueryRet = SQLExecDirect(_Statement, (SQLWCHAR*)Query, SQL_NTSL);
	if (QueryRet == SQL_SUCCESS || QueryRet == SQL_SUCCESS_WITH_INFO)
	{
		return true;
	}

	DBError(QueryRet);

	return false;
}

// 쿼리 완료후 데이터 가져올때 필요한 함수
bool CDBConnection::Fetch()
{
	SQLRETURN Ret = SQLFetch(_Statement);

	switch (Ret)
	{
	case SQL_SUCCESS:
	case SQL_SUCCESS_WITH_INFO:
		return true;
	case SQL_NO_DATA: // 쿼리는 완료 됏지만 반환하는 값은 없을 경우
		return false;
	case SQL_ERROR:
		DBError(Ret);
		return false;
	}
}

int32 CDBConnection::GetRowCount()
{
	SQLLEN Count = 0;
	SQLRETURN Ret = SQLRowCount(_Statement, OUT & Count);

	if (Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO)
	{
		return (int32)(Count);
	}

	return -1;
}

// 이전에 받앗던 DB 결과물을 제거
void CDBConnection::UnBind()
{  
	// Statement 정리
	SQLFreeStmt(_Statement, SQL_UNBIND);
	SQLFreeStmt(_Statement, SQL_RESET_PARAMS);
	SQLFreeStmt(_Statement, SQL_CLOSE);
}

bool CDBConnection::BindParam(int32 ParamIndex, bool* Value, SQLLEN* Index)
{
	return BindParam(ParamIndex, SQL_C_TINYINT, SQL_TINYINT, size32(bool), Value, Index);
}

bool CDBConnection::BindParam(int32 ParamIndex, float* Value, SQLLEN* Index)
{
	return BindParam(ParamIndex, SQL_C_FLOAT, SQL_REAL, 0, Value, Index);
}

bool CDBConnection::BindParam(int32 ParamIndex, double* Value, SQLLEN* Index)
{
	return BindParam(ParamIndex, SQL_C_DOUBLE, SQL_DOUBLE, 0, Value, Index);
}

bool CDBConnection::BindParam(int32 ParamIndex, int8* Value, SQLLEN* Index)
{
	return BindParam(ParamIndex, SQL_C_TINYINT, SQL_TINYINT, size32(int8), Value, Index);
}

bool CDBConnection::BindParam(int32 ParamIndex, int16* Value, SQLLEN* Index)
{
	return BindParam(ParamIndex, SQL_C_SHORT, SQL_SMALLINT, size32(int16), Value, Index);
}

bool CDBConnection::BindParam(int32 ParamIndex, int32* Value, SQLLEN* Index)
{
	return BindParam(ParamIndex, SQL_C_LONG, SQL_INTEGER, size32(int32), Value, Index);
}

bool CDBConnection::BindParam(int32 ParamIndex, int64* Value, SQLLEN* Index)
{
	return BindParam(ParamIndex, SQL_C_SBIGINT, SQL_BIGINT, size32(int64), Value, Index);
}

bool CDBConnection::BindParam(int32 ParamIndex, TIMESTAMP_STRUCT* TimeValue, SQLLEN* Index)
{
	return BindParam(ParamIndex, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, size32(TIMESTAMP_STRUCT), TimeValue, Index);
}

bool CDBConnection::BindParam(int32 ParamIndex, const WCHAR* Str, SQLLEN* Index)
{
	SQLULEN Size = (SQLULEN)(wcslen(Str + 1) * 2);
	*Index = SQL_NTSL;

	if (Size > WVARCHAR_MAX)
	{
		return BindParam(ParamIndex, SQL_C_WCHAR, SQL_WLONGVARCHAR, Size, (SQLPOINTER)Str, Index);
	}
	else
	{
		return BindParam(ParamIndex, SQL_C_WCHAR, SQL_WVARCHAR, Size, (SQLPOINTER)Str, Index);
	}
}

bool CDBConnection::BindParam(int32 ParamIndex, const char* Str, SQLLEN* Index)
{
	SQLULEN Size = (SQLULEN)(strlen(Str + 1));
	*Index = SQL_NTSL;

	if (Size > CHAR_MAX)
	{
		return BindParam(ParamIndex, SQL_C_CHAR, SQL_LONGVARCHAR, Size, (SQLPOINTER)Str, Index);
	}
	else
	{
		return BindParam(ParamIndex, SQL_C_CHAR, SQL_VARCHAR, Size, (SQLPOINTER)Str, Index);
	}	
}

bool CDBConnection::BindParam(int32 ParamIndex, const BYTE* Binary, int32 Size, SQLLEN* Index)
{
	if (Binary == nullptr)
	{
		*Index = SQL_NULL_DATA;
		Size = 1;
	}
	else
	{
		*Index = Size;
	}

	if (Size > BINARY_MAX)
	{
		return BindParam(ParamIndex, SQL_C_BINARY, SQL_LONGVARBINARY, Size, (BYTE*)Binary, Index);
	}
	else
	{
		return BindParam(ParamIndex, SQL_C_BINARY, SQL_BINARY, Size, (BYTE*)Binary, Index);
	}
}

bool CDBConnection::BindCol(int32 ColumnIndex, bool* Value, SQLLEN* Index)
{
	return BindCol(ColumnIndex, SQL_C_TINYINT, size32(bool), Value, Index);
}

bool CDBConnection::BindCol(int32 ColumnIndex, float* Value, SQLLEN* Index)
{
	return BindCol(ColumnIndex, SQL_C_FLOAT, size32(float), Value, Index);
}

bool CDBConnection::BindCol(int32 ColumnIndex, double* Value, SQLLEN* Index)
{
	return BindCol(ColumnIndex, SQL_C_DOUBLE, size32(double), Value, Index);
}

bool CDBConnection::BindCol(int32 ColumnIndex, int8* Value, SQLLEN* Index)
{
	return BindCol(ColumnIndex, SQL_C_TINYINT, size32(int8), Value, Index);
}

bool CDBConnection::BindCol(int32 ColumnIndex, int16* Value, SQLLEN* Index)
{
	return BindCol(ColumnIndex, SQL_C_SHORT, size32(int16), Value, Index);
}

bool CDBConnection::BindCol(int32 ColumnIndex, int32* Value, SQLLEN* Index)
{
	return BindCol(ColumnIndex, SQL_C_LONG, size32(int32), Value, Index);
}

bool CDBConnection::BindCol(int32 ColumnIndex, int64* Value, SQLLEN* Index)
{
	return BindCol(ColumnIndex, SQL_C_SBIGINT, size32(int64), Value, Index);
}

bool CDBConnection::BindCol(int32 ColumnIndex, TIMESTAMP_STRUCT* Value, SQLLEN* Index)
{
	return BindCol(ColumnIndex, SQL_C_TYPE_TIMESTAMP, size32(TIMESTAMP_STRUCT), Value, Index);
}

bool CDBConnection::BindCol(int32 ColumnIndex, WCHAR* Str, int32 Size, SQLLEN* Index)
{
	return BindCol(ColumnIndex, SQL_C_WCHAR, Size, Str, Index);
}

bool CDBConnection::BindCol(int32 ColumnIndex, BYTE* Binary, int32 Size, SQLLEN* Index)
{
	return BindCol(ColumnIndex, SQL_BINARY, Size, Binary, Index);
}

bool CDBConnection::BindParam(SQLUSMALLINT ParamIndex, SQLUSMALLINT CType, SQLSMALLINT SqlType, SQLULEN Length, SQLPOINTER Ptr, SQLLEN* Index)
{
	SQLRETURN Ret = SQLBindParameter(_Statement, ParamIndex, SQL_PARAM_INPUT, CType, SqlType, Length, 0, Ptr, 0, Index);
	if (Ret != SQL_SUCCESS && Ret != SQL_SUCCESS_WITH_INFO)
	{
		DBError(Ret);
		return false;
	}

	return true;
}

bool CDBConnection::BindCol(SQLUSMALLINT ColumnIndex, SQLSMALLINT CType, SQLULEN Length, SQLPOINTER Value, SQLLEN* Index)
{
	SQLRETURN Ret = SQLBindCol(_Statement, ColumnIndex, CType, Value, Length, Index);
	if (Ret != SQL_SUCCESS && Ret != SQL_SUCCESS_WITH_INFO)
	{
		DBError(Ret);
		return false;
	}

	return true;
}

void CDBConnection::DBError(SQLRETURN Ret)
{
	if (Ret == SQL_SUCCESS)
	{
		return;
	}

	SQLSMALLINT Index = 1;
	SQLWCHAR SqlState[MAX_PATH] = { 0 };
	SQLINTEGER NativeError = 0;
	SQLWCHAR ErrorMsg[MAX_PATH] = { 0 };
	SQLSMALLINT MsgLength = 0;
	SQLRETURN ErrorRet = 0;

	while (true)
	{
		ErrorRet = SQLGetDiagRec(
			SQL_HANDLE_STMT,
			_Statement,
			Index,
			SqlState,
			OUT & NativeError,
			ErrorMsg,
			_countof(ErrorMsg),
			OUT & MsgLength);

		if (ErrorRet == SQL_NO_DATA)
		{
			break;
		}

		if (ErrorRet != SQL_SUCCESS && ErrorRet != SQL_SUCCESS_WITH_INFO)
		{
			break;
		}

		// 개발 단계에서는 콘솔로 찍지만, 보통 파일로 남기거나 DB에 저장
		wcout.imbue(locale("kor"));
		wcout << ErrorMsg << endl;

		Index++;
	}
}
