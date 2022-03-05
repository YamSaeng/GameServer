#pragma once
#include"pch.h"
#include"Message.h"

enum class en_LoginServerJobType : int16
{
	AUTH_MESSAGE = 1,
	AUTH_ACCOUNT_NEW,
	AUTH_ACCOUNT_LOGIN,
	AUTH_ACCOUNT_LOGOUT,
	AUTH_ACCOUNT_DISCONNECT,

	DATA_BASE_ACCOUNT_NEW,
	DATA_BASE_ACCOUNT_LOGIN,
	DATA_BASE_ACCOUNT_LOGOUT,
	DATA_BASE_ACCOUNT_DISCONNECT
};

struct st_LoginServerJob
{
	en_LoginServerJobType Type;
	int64 SessionID;	
	CMessage* Message;
};