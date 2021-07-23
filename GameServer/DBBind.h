#pragma once
#include "DBConnection.h"
#include "Type.h"

//-------------------------------------------------------------------------------
// CDBBind
// 
// BindParam (���� ����� �ʿ��� ���ڸ� �Ѱ��ٶ� ���)
// BindCol   (���� ���� �� ���õ� �����͸� ������ ���)
// ���� 2������ �����Ҷ� �Ǽ��� �����Ͽ� ������ �߻��ϴ°��� �����ϱ� ���� Ŭ����
//-------------------------------------------------------------------------------

// �Է¹��� Count ������ŭ �������� �о ������ش�.
// FullBits<3>::Value �̷��� �Է��ϸ�
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

// ParamCount : ���ε��� Param���� ���� ( = ���� ������ )
// ColumnCount : ���� ���� �� ���� Column ���� ( = �̾ƿ� ������ )
template<int32 ParamCount, int32 ColumnCount>
class CDBBind
{
protected:
	// DBConnection
	CDBConnection& _DBConnection;
	// ����
	const WCHAR* _Query;

	// �Է¹��� ParamCount�� ���� SQLLen�� ������ �����ش�.
	SQLLEN _ParamIndex[ParamCount > 0 ? ParamCount : 1];
	SQLLEN _ColumnIndex[ColumnCount > 0 ? ColumnCount : 1];

	// DBBind 24:00	
	uint64 _ParamFlag;  // �Է¹��� ParamCount ��ŭ ������ ���ε� �ߴ��� Ȯ��
	uint64 _ColumnFlag; // �Է¹��� ColumnCount ��ŭ ������ �����͸� ���� �Դ��� Ȯ��
	// 64��Ʈ ������ Ȱ���ؼ� ī���� �Ѵ�.
	// ��, [][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][]
	// BindParam, BindCol �Ҷ����� 1�� ������ �ڸ��� ����־ ī����	
	// ex) ���� CDBBind<2,0>�� �����ϰ�
	// BindParam�� ���Ҷ����� 1�� ���� �ִ´�.
	// _ParamFlag = [1][1][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][]
	// �̷� ��������
public:
	CDBBind(CDBConnection& DBConnection, const WCHAR* Query) : _DBConnection(DBConnection), _Query(Query)
	{
		memset(_ParamIndex, 0, sizeof(_ParamIndex));
		memset(_ColumnIndex, 0, sizeof(_ColumnIndex));

		_ParamFlag = 0;
		_ColumnFlag = 0;

		DBConnection.UnBind();
	}

	// �Է¹��� _ParamCount, ColumnCount ��ŭ BindParam, BindCol �ߴ��� Ȯ���Ѵ�.
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

	// ������ �����Ѵ�.
	bool Execute()
	{
		ASSERT_CRASH(Validate());

		return _DBConnection.Execute(_Query);
	}

	// SQL Fetch�� �̿��� �����͸� �����´�.
	bool Fetch()
	{
		return _DBConnection.Fetch();
	}

#pragma region BindParam
	// ���� BindParam �Լ�

	// �Ϲ� ���� �ֱ�
	template<typename T>
	void BindParam(int32 Index, T& Value)
	{
		_DBConnection.BindParam(Index + 1, &Value, &_ParamIndex[Index]);
		_ParamFlag |= (1LL << Index);
	}

	// ���ڿ� �ֱ�
	void BindParam(int32 Index, const WCHAR* Value)
	{
		_DBConnection.BindParam(Index + 1, Value, &_ParamIndex[Index]);
		_ParamFlag |= (1LL << Index);
	}

	// �Ϲ����� �迭 ���ε��ϱ�
	template<typename T, int32 N>
	void BindParam(int32 Index, T(&Value)[N])
	{
		_DBConnection.BindParam(Index + 1, (const BYTE*)Value, size32(T) * N, &_ParamIndex[Index]);
		_ParamFlag |= (1LL << Index);
	}

	// ������ �������� N ���ϱ� ��ŭ ������ ���ε�
	template<typename T>
	void BindParam(int32 Index, T* Value, int32 N)
	{
		_DBConnection.BindParam(Index + 1, (const BYTE*)Value, size32(T) * N, &_ParamIndex[Index]);
		_ParamFlag |= (1LL << Index);
	}
#pragma endregion	

#pragma region BindCol
	// ���� BindCol �Լ�

	template<typename T>
	void BindCol(int32 Index, T& Value)
	{
		_DBConnection.BindCol(Index + 1, &Value, &_ColumnIndex[Index]);
		_ColumnFlag |= (1LL << Index);
	}

	// WCHAR �迭
	template<int32 N>
	void BindCol(int32 Index, WCHAR(&Value)[N])
	{
		// NULL ���� ����
		_DBConnection.BindCol(Index + 1, Value, N - 1, &_ColumnIndex[Index]);
		_ColumnFlag |= (1LL << Index);
	}

	// WCHAR �����Ϳ� ���� ����
	void BindCol(int32 Index, WCHAR* Value, int32 Length)
	{
		_DBConnection.BindCol(Index + 1, Value, Length - 1, &_ColumnIndex[Index]);
		_ColumnFlag |= (1LL << Index);
	}

	// �Ϲ����� �迭 
	template<typename T, int32 N>
	void BindCol(int32 Index, T(&Value)[N])
	{
		_DBConnection.BindCol(Index + 1, Value, size32(T) * N, &_ColumnIndex[Index]);
		_ColumnFlag |= (1LL << Index);
	}
#pragma endregion	

};