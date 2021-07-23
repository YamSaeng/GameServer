#include "pch.h"
#include "FileUtils.h"
#include <filesystem>
#include <fstream>

/*-----------------
	FileUtils
------------------*/

namespace FS = std::filesystem;

vector<BYTE> FileUtils::ReadFile(const WCHAR* path)
{
	vector<BYTE> Ret;
	
	// File Path ����
	FS::path FilePath{ path };

	// FileSize ���ϱ�
	const uint32 FileSize = static_cast<uint32>(FS::file_size(FilePath));
	// FileSize ��ŭ vectorũ�� ����
	Ret.resize(FileSize);

	// FilePath�� �ִ� ���� ������ �о
	// vector Ret�� ����
	basic_ifstream<BYTE> InputStream{ FilePath };
	InputStream.read(&Ret[0], FileSize);

	return Ret;
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
