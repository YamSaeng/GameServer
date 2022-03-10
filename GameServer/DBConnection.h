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
	// SQL 연결 핸들
	SQLHDBC _DBConnectin = SQL_NULL_HANDLE;
	SQLHSTMT _Statement = SQL_NULL_HANDLE;
public:
	// 입력받은 ConnectionString을 통해 DB에 연결
	bool Connect(SQLHENV Henv, const WCHAR* ConnectionString);
	// 핸들 정리
	void Clear();

	// 입력받은 쿼리문 실행
	bool Execute(const WCHAR* Query);
	// 쿼리 완료후 데이터 가져올때 필요한 함수	
	bool Fetch();
	// 행 개수 반환
	int32 GetRowCount();

	// 이전에 받앗두었던 DB 결과물들을 정리
	// (Statement를 정리한것과 동일)
	void UnBind();

	// 쿼리 실행할때 넘겨주는 인자를 바인딩 할때 사용
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

	// 쿼리 실행후 관련된 데이터를 받아올때 사용할 함수
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
	// SQL 쿼리를 실행할때 인자를 넘겨줘야하는 경우가 자주 생기는데, SQL 쿼리한테 _Statement를 통해 인자를 넘겨줄때 
	// 관련된 함수	
	bool BindParam(SQLUSMALLINT ParamIndex, SQLUSMALLINT CType, SQLSMALLINT SqlType, SQLULEN Length, SQLPOINTER Ptr, SQLLEN* Index);
	// 쿼리 날리고 나서 관련된 데이터를 받아올때 사용할 함수
	bool BindCol(SQLUSMALLINT ColumnIndex, SQLSMALLINT CType, SQLULEN Length, SQLPOINTER Value, SQLLEN* Index);
	// 에러발생시 호출
	void DBError(SQLRETURN Ret);
};

