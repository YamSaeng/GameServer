#pragma once
#include <winnt.h>
#include <minwindef.h>
#include "Message.h"

enum en_MESSAGE_TYPE
{
	NEW_CLIENT_JOIN = 0,
	DISCONNECT_CLIENT = 1,
	MESSAGE = 2,

	SECTOR_X_MAX = 50,
	SECTOR_Y_MAX = 50,

	LOGIN_SUCCESS = 1,
	LOGIN_FAIL = 0,
};

struct st_CLIENT
{
	__int64 AccountNo;
	__int64 SessionID;

	WCHAR ClientID[20];
	WCHAR NickName[20];

	short SectorX;
	short SectorY;

	char SessionKey[64];

	bool IsLogin;

	DWORD RecvPacketTime;
};

struct st_SECTOR_POSITION
{
	int X;
	int Y;
};

struct st_SECTOR_AROUND
{
	int Count;
	st_SECTOR_POSITION Around[9];
};

struct st_JOB
{
	WORD Type;
	__int64 SessionID;
	CMessage* Message;
};


//논블락소켓 셀렉트로 랜클라이언트 구현
//