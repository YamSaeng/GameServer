#pragma once
#include"pch.h"
#include"Message.h"

#define TOKEN_EXPIRED_TIME 600
#define KST_TIME 32400
#define UTC_YAER 1900
#define UTC_MONTH 1
#define TOKEN_MAX_LENGTH 50

// 로그인 정보
enum class en_LoginInfo : int8
{
	LOGIN_ACCOUNT_NOT_EXIST,
	LOGIN_ACCOUNT_OVERLAP,
	LOGIN_ACCOUNT_DIFFERENT_PASSWORD,
	LOGIN_ACCOUNT_LOGIN_SUCCESS
};

// 로그인 상태
enum class en_LoginState : int8
{
	LOGIN_OUT,
	LOGIN_IN
};

// 로그인 서버 잡 타입
enum class en_LoginServerJobType : int16
{
	AUTH_MESSAGE = 1,	

	DATA_BASE_ACCOUNT_NEW,
	DATA_BASE_ACCOUNT_LOGIN,
	DATA_BASE_ACCOUNT_LOGOUT,
	DATA_BASE_ACCOUNT_DISCONNECT
};

// 로그인 서버 잡 구조체
struct st_LoginServerJob
{
	en_LoginServerJobType Type;
	int64 SessionID;	
	CMessage* Message;
};

// 서버 정보
struct st_ServerInfo
{
	wstring ServerName;
	wstring ServerIP;
	int32 ServerPort;
	float ServerBusy;
};