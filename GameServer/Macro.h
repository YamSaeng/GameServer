#pragma once

#pragma region Crash ����

// __analysis_assume(Crash != nullptr); �����Ϸ����� Crash�� null�� �ƴ϶�� ���δ�
// �׸��� �Ʒ����� ���⿡�� �߻�������
#define CRASH(cause)						\
{											\
	uint32* Crash = nullptr;				\
	__analysis_assume(Crash != nullptr);	\
	*Crash = 0xDEADBEEF;					\
}											\

#define ASSERT_CRASH(Expr)					\
{											\
	if(!(Expr))								\
	{										\
		CRASH("ASSERT CRASH");				\
		__analysis_assume(Expr);			\
	}										\
}											\

#pragma endregion

// ���� ������ ���ϱ�
#define size16(val)	static_cast<int16>(sizeof(val))
#define size32(val)	static_cast<int32>(sizeof(val))
// �迭 ������ ���ϱ�
#define len16(arr)	static_cast<int16>(sizeof(arr)/sizeof(arr[0]))
#define len32(arr)  static_cast<int32>(sizeof(arr)/sizeof(arr[0]))