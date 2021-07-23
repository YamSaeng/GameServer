#pragma once
#include <corecrt_wstdio.h>
#include "Type.h"

// ÄÜ¼Ö »ö
enum class en_Color
{
	BLACK,
	WHITE,
	RED,
	GREEN,
	BLUE,
	YELLOW,
};

class CLog
{
	enum { BUFFER_SIZE = 4096 };
private:
	HANDLE _StdOut;
	HANDLE _StdErr;
protected:
	void SetColor(bool StdOut, en_Color Color);

public:
	CLog();
	~CLog();

	void WriteStdOut(en_Color Color, const WCHAR* Format, ...);
	void WriteStdErr(en_Color Color, const WCHAR* Format, ...);
};