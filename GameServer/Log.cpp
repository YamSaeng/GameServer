#include "pch.h"
#include "Log.h"

CLog::CLog()
{
	_StdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
	_StdErr = ::GetStdHandle(STD_ERROR_HANDLE);
}

CLog::~CLog()
{
}

void CLog::SetColor(bool StdOut, en_Color Color)
{
	static WORD SColors[]
	{
		0,
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
		FOREGROUND_RED | FOREGROUND_INTENSITY,
		FOREGROUND_GREEN | FOREGROUND_INTENSITY,
		FOREGROUND_BLUE | FOREGROUND_INTENSITY,
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY
	};

	::SetConsoleTextAttribute(StdOut ? _StdOut : _StdErr, SColors[static_cast<int32>(Color)]);
}

void CLog::WriteStdOut(en_Color Color, const WCHAR* Format, ...)
{
	if (Format == nullptr)
	{
		return;
	}

	SetColor(true, Color);

	va_list FormatList;
	va_start(FormatList, Format);
	::vwprintf(Format, FormatList);
	va_end(FormatList);

	fflush(stdout);

	SetColor(true, en_Color::WHITE);
}

void CLog::WriteStdErr(en_Color Color, const WCHAR* Format, ...)
{
	WCHAR Buffer[BUFFER_SIZE];

	if (Format == nullptr)
	{
		return;
	}

	SetColor(false, Color);

	va_list FormatList;
	va_start(FormatList, Format);
	::vswprintf_s(Buffer, BUFFER_SIZE, Format, FormatList);
	va_end(FormatList);

	::fwprintf_s(stderr, Buffer);
	fflush(stderr);

	SetColor(false, en_Color::WHITE);
}
