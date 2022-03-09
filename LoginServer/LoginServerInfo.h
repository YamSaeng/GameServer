#pragma once
#include"pch.h"
#include"Message.h"

#define TOKEN_EXPIRED_TIME 600
#define KST_TIME 32400
#define UTC_YAER 1900
#define UTC_MONTH 1
#define TOKEN_MAX_LENGTH 50

// �α��� ����
enum class en_LoginInfo : int8
{
	LOGIN_ACCOUNT_NOT_EXIST,
	LOGIN_ACCOUNT_OVERLAP,
	LOGIN_ACCOUNT_DIFFERENT_PASSWORD,
	LOGIN_ACCOUNT_LOGIN_SUCCESS
};

// �α��� ����
enum class en_LoginState : int8
{
	LOGIN_OUT,
	LOGIN_IN
};

// �α��� ���� �� Ÿ��
enum class en_LoginServerJobType : int16
{
	AUTH_MESSAGE = 1,	

	DATA_BASE_ACCOUNT_NEW,
	DATA_BASE_ACCOUNT_LOGIN,
	DATA_BASE_ACCOUNT_LOGOUT,
	DATA_BASE_ACCOUNT_DISCONNECT
};

// �α��� ���� �� ����ü
struct st_LoginServerJob
{
	en_LoginServerJobType Type;
	int64 SessionID;	
	CMessage* Message;
};

// ���� ����
struct st_ServerInfo
{
	wstring ServerName;
	wstring ServerIP;
	int32 ServerPort;
	float ServerBusy;
};