#include "pch.h"
#include "FileUtils.h"

// 파일 읽어서 안에 있는 내용 반환 
char* FileUtils::LoadFile(const wchar_t* Path)
{
	char RetBuf[100000];
	memset(RetBuf, 0, sizeof(RetBuf));

	DWORD ReadSize;

	HANDLE File = CreateFile(Path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (File == INVALID_HANDLE_VALUE)
	{
		printf("파일 안열림");
	}

	int DataSize = GetFileSize(File, NULL);

	ReadFile(File, RetBuf, DataSize, &ReadSize, NULL);
	if (ReadSize != DataSize)
	{
		CloseHandle(File);
		return nullptr;
	}

	CloseHandle(File);

	return RetBuf;
}

wstring FileUtils::Convert(string str)
{
	const int32 srcLen = static_cast<int32>(str.size());

	wstring ret;
	if (srcLen == 0)
		return ret;
		
	const int32 retLen = ::MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(&str[0]), srcLen, NULL, 0);
	ret.resize(retLen);
	::MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(&str[0]), srcLen, &ret[0], retLen);

	return ret;
}
