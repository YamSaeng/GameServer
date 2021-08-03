#pragma once
#include <winnt.h>
#include <minwindef.h>
#include "Message.h"
#include "Player.h"

enum en_MESSAGE_TYPE
{
	NEW_CLIENT_JOIN = 0,
	DISCONNECT_CLIENT = 1,
	MESSAGE = 2,

	SECTOR_X_MAX = 50,
	SECTOR_Y_MAX = 50,

	LOGIN_SUCCESS = 1,
	LOGIN_FAIL = 0,

	DATA_BASE_ACCOUNT_CHECK = 100,
	DATA_BASE_CHARACTER_CHECK,
};

struct st_CLIENT
{
	int64 AccountID;
	int64 SessionID;

	WCHAR ClientID[20];
	wstring CreateCharacterName;

	int16 SectorX;
	int16 SectorY;

	int32 Token;

	bool IsLogin;

	DWORD RecvPacketTime;
	
	CPlayer MyPlayers[5];
};

struct st_SECTOR_POSITION
{
	int32 X;
	int32 Y;
};

struct st_SECTOR_AROUND
{
	int32 Count;
	st_SECTOR_POSITION Around[9];
};

struct st_JOB
{
	WORD Type;
	int64 SessionID;
	CMessage* Message;
};


//논블락소켓 셀렉트로 랜클라이언트 구현
//