#pragma once

#pragma region Crash 관련

// __analysis_assume(Crash != nullptr); 컴파일러에게 Crash가 null이 아니라고 속인다
// 그리고 아래에서 쓰기에러 발생시켜줌
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

// 변수 사이즈 구하기
#define size16(val)	static_cast<int16>(sizeof(val))
#define size32(val)	static_cast<int32>(sizeof(val))
// 배열 사이즈 구하기
#define len16(arr)	static_cast<int16>(sizeof(arr)/sizeof(arr[0]))
#define len32(arr)  static_cast<int32>(sizeof(arr)/sizeof(arr[0]))