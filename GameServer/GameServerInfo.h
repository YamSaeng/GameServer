#pragma once
#include <winnt.h>
#include <minwindef.h>
#include "Message.h"
#include "Player.h"

#define SECTOR_X_MAX 50
#define SECTOR_Y_MAX 50

#define LOGIN_SUCCESS 1
#define LOGIN_FAIL 0

enum class en_MESSAGE_TYPE : int16
{
	AUTH_NEW_CLIENT_JOIN = 0,
	AUTH_DISCONNECT_CLIENT = 1,
	AUTH_MESSAGE = 2,

	NETWORK_MESSAGE = 3,	

	DATA_BASE_ACCOUNT_CHECK = 100,
	DATA_BASE_CHARACTER_CHECK,	
	DATA_BASE_ITEM_CREATE,
	DATA_BASE_ITEM_INVENTORY_SAVE,
	DATA_BASE_ITEM_SWAP,
	DATA_BASE_GOLD_SAVE,
	DATA_BASE_CHARACTER_INFO_SEND
};

struct st_Job
{
	en_MESSAGE_TYPE Type;
	int64 SessionId;
	CMessage* Message;
	st_SESSION* Session;
};


//논블락소켓 셀렉트로 랜클라이언트 구현
//