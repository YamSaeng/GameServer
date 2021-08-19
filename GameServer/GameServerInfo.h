#pragma once
#include <winnt.h>
#include <minwindef.h>
#include "Message.h"
#include "Player.h"

enum en_MESSAGE_TYPE
{
	AUTH_NEW_CLIENT_JOIN = 0,
	AUTH_DISCONNECT_CLIENT = 1,
	AUTH_MESSAGE = 2,

	MESSAGE = 3,

	SECTOR_X_MAX = 50,
	SECTOR_Y_MAX = 50,

	LOGIN_SUCCESS = 1,
	LOGIN_FAIL = 0,

	DATA_BASE_ACCOUNT_CHECK = 100,
	DATA_BASE_CHARACTER_CHECK,
};

struct st_CLIENT
{
	int64 AccountId;
	int64 SessionId;

	wstring ClientId;
	wstring CreateCharacterName;

	int16 SectorX;
	int16 SectorY;

	int32 Token;

	bool IsLogin;

	DWORD RecvPacketTime;
	
	CPlayer* MyPlayers[5];
	CPlayer* MyPlayer;
};

struct st_Job
{
	WORD Type;
	int64 SessionID;
	CMessage* Message;
};


//�������� ����Ʈ�� ��Ŭ���̾�Ʈ ����
//