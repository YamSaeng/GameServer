#pragma once
#include "Type.h"

/*-----------------
	FileUtils
------------------*/

class FileUtils
{
public:
	// 경로에 있는 파일을 읽어 들여서 배열에 저장후 반환
	static vector<BYTE>		ReadFile(const WCHAR* path);
	// 입력받은 string을 wstring으로 변환해서 반환
	static wstring			Convert(string str);
};