#pragma once
#include "Type.h"

/*-----------------
	FileUtils
------------------*/

class FileUtils
{
public:
	// ��ο� �ִ� ������ �о� �鿩�� �迭�� ������ ��ȯ
	static char* LoadFile(const wchar_t* Path);
	// �Է¹��� string�� wstring���� ��ȯ�ؼ� ��ȯ
	static wstring Convert(string str);
};