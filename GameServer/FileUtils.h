#pragma once
#include "Type.h"

/*-----------------
	FileUtils
------------------*/

class FileUtils
{
public:
	// ��ο� �ִ� ������ �о� �鿩�� �迭�� ������ ��ȯ
	static vector<BYTE>		ReadFile(const WCHAR* path);
	// �Է¹��� string�� wstring���� ��ȯ�ؼ� ��ȯ
	static wstring			Convert(string str);
};