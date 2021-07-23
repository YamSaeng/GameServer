#pragma once
#include "DBConnection.h"
#include "Type.h"

//-------------------------------------------------------------------------------
// CDBBind
// 
// BindParam (쿼리 실행시 필요한 인자를 넘겨줄때 사용)
// BindCol   (쿼리 실행 후 관련된 데이터를 받을떄 사용)
// 위의 2가지를 실행할때 실수로 누락하여 에러가 발생하는것을 방지하기 위한 클래스
//-------------------------------------------------------------------------------

// 입력받은 Count 개수만큼 왼쪽으로 밀어서 기록해준다.
// FullBits<3>::Value 이렇게 입력하면
// Value = ( 1 << (3 - 1)) | FullBits<3 - 1>::Value
// Value = ( 1 << (3 - 1)) | Value = ( 1 << (2 - 1)) | FullBits<2 - 1>::Value
// Value = ( 1 << (3 - 1)) | Value = ( 1 << (2 - 1)) | FullBits<1>::Value | FullBits<1 - 1>::Value
// Value = ( 1 << (3 - 1)) | Value = ( 1 << (2 - 1)) | FullBits<1>::Value | FullBits<0>::Value
// Value = ( 1 << (3 - 1)) | Value = ( 1 << (2 - 1)) | 1 | 0
// -> [1][1][1]

template<int32 Count>
struct FullBits { enum { Value = (1 << (Count - 1)) | FullBits<Count - 1>::Value }; };
template<>
struct FullBits<1> { enum { Value = 1 }; };
template<>
struct FullBits<0> { enum { Value = 0 }; };

// ParamCount : 바인딩할 Param값의 개수 ( = 넣을 데이터 )
// ColumnCount : 쿼리 질의 후 받을 Column 개수 ( = 뽑아올 데이터 )
template<int32 ParamCount, int32 ColumnCount>
class CDBBind
{
protected:
	// DBConnection
	CDBConnection& _DBConnection;
	// 쿼리
	const WCHAR* _Query;

	// 입력받은 ParamCount에 따라 SQLLen의 개수를 정해준다.
	SQLLEN _ParamIndex[ParamCount > 0 ? ParamCount : 1];
	SQLLEN _ColumnIndex[ColumnCount > 0 ? ColumnCount : 1];

	// DBBind 24:00	
	uint64 _ParamFlag;  // 입력받은 ParamCount 만큼 실제로 바인딩 했는지 확인
	uint64 _ColumnFlag; // 입력받은 ColumnCount 만큼 실제로 데이터를 가져 왔는지 확인
	// 64비트 변수를 활용해서 카운팅 한다.
	// 즉, [][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][]
	// BindParam, BindCol 할때마다 1을 각각의 자리에 집어넣어서 카운팅	
	// ex) 만약 CDBBind<2,0>로 선언하고
	// BindParam을 콜할때마다 1씩 집어 넣는다.
	// _ParamFlag = [1][1][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][]
	// 이런 형식으로
public:
	CDBBind(CDBConnection& DBConnection, const WCHAR* Query) : _DBConnection(DBConnection), _Query(Query)
	{
		memset(_ParamIndex, 0, sizeof(_ParamIndex));
		memset(_ColumnIndex, 0, sizeof(_ColumnIndex));

		_ParamFlag = 0;
		_ColumnFlag = 0;

		DBConnection.UnBind();
	}

	// 입력받은 _ParamCount, ColumnCount 만큼 BindParam, BindCol 했는지 확인한다.
	bool Validate()
	{
		if (_ParamFlag == FullBits<ParamCount>::Value && _ColumnFlag == FullBits<ColumnCount>::Value)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	// 쿼리를 실행한다.
	bool Execute()
	{
		ASSERT_CRASH(Validate());

		return _DBConnection.Execute(_Query);
	}

	// SQL Fetch를 이용해 데이터를 가져온다.
	bool Fetch()
	{
		return _DBConnection.Fetch();
	}

#pragma region BindParam
	// 각종 BindParam 함수

	// 일반 변수 넣기
	template<typename T>
	void BindParam(int32 Index, T& Value)
	{
		_DBConnection.BindParam(Index + 1, &Value, &_ParamIndex[Index]);
		_ParamFlag |= (1LL << Index);
	}

	// 문자열 넣기
	void BindParam(int32 Index, const WCHAR* Value)
	{
		_DBConnection.BindParam(Index + 1, Value, &_ParamIndex[Index]);
		_ParamFlag |= (1LL << Index);
	}

	// 일반적인 배열 바인딩하기
	template<typename T, int32 N>
	void BindParam(int32 Index, T(&Value)[N])
	{
		_DBConnection.BindParam(Index + 1, (const BYTE*)Value, size32(T) * N, &_ParamIndex[Index]);
		_ParamFlag |= (1LL << Index);
	}

	// 포인터 기준으로 N 곱하기 만큼 데이터 바인딩
	template<typename T>
	void BindParam(int32 Index, T* Value, int32 N)
	{
		_DBConnection.BindParam(Index + 1, (const BYTE*)Value, size32(T) * N, &_ParamIndex[Index]);
		_ParamFlag |= (1LL << Index);
	}
#pragma endregion	

#pragma region BindCol
	// 각종 BindCol 함수

	template<typename T>
	void BindCol(int32 Index, T& Value)
	{
		_DBConnection.BindCol(Index + 1, &Value, &_ColumnIndex[Index]);
		_ColumnFlag |= (1LL << Index);
	}

	// WCHAR 배열
	template<int32 N>
	void BindCol(int32 Index, WCHAR(&Value)[N])
	{
		// NULL 문자 제외
		_DBConnection.BindCol(Index + 1, Value, N - 1, &_ColumnIndex[Index]);
		_ColumnFlag |= (1LL << Index);
	}

	// WCHAR 포인터와 길이 받음
	void BindCol(int32 Index, WCHAR* Value, int32 Length)
	{
		_DBConnection.BindCol(Index + 1, Value, Length - 1, &_ColumnIndex[Index]);
		_ColumnFlag |= (1LL << Index);
	}

	// 일반적인 배열 
	template<typename T, int32 N>
	void BindCol(int32 Index, T(&Value)[N])
	{
		_DBConnection.BindCol(Index + 1, Value, size32(T) * N, &_ColumnIndex[Index]);
		_ColumnFlag |= (1LL << Index);
	}
#pragma endregion	

};